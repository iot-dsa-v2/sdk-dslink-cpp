#include "dsa_common.h"

#include "tcp_connection.h"

#include <boost/bind.hpp>

#include "session_manager.h"

namespace dsa {

TcpConnection::TcpConnection(const App &app, const Config &config)
    : Connection(app, config), _socket(app.io_service()) {}

void TcpConnection::close() {
  if (_socket_open.exchange(false)) {
    // call n times to make sure socket operations are all cancelled
    for (unsigned int i = 0; i < 5; ++i)
      _socket.close();
  }
  // call n times to make sure timer operations are all cancelled
  for (unsigned int i = 0; i < 5; ++i)
    _deadline.cancel();
}

void TcpConnection::read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) {
  // reset deadline timer for each new message
  reset_standard_deadline_timer();

  if (!error) {
    std::cout << std::endl << "in read loop" << std::endl;

    BufferPtr buf = std::move(_read_buffer);
    _read_buffer.reset(new Buffer());

    size_t total_bytes = from_prev + bytes_transferred;
    uint8_t *data = buf->data();
    size_t cur = 0;

    while (cur < total_bytes) {
      // always want full static header instead of just message size to make sure it's a valid message
      if (total_bytes - cur < StaticHeaders::TotalSize) {
        size_t partial_size = total_bytes - cur;
        _read_buffer->assign(&data[cur], partial_size);
        _socket.async_read_some(boost::asio::buffer(_read_buffer->data() + partial_size,
                                                    _read_buffer->capacity() - partial_size),
                                boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), partial_size,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
      }

      StaticHeaders header(&data[cur]);
      if (header.message_size < total_bytes - cur) {
        size_t partial_size = total_bytes - cur;

        // make sure buffer capacity is enough to read full message
        _read_buffer->resize(header.message_size);
        _read_buffer->assign(&data[cur], partial_size);

        // read the rest of the message
        boost::asio::async_read(_socket,
                                boost::asio::buffer(_read_buffer->data() + partial_size,
                                                    _read_buffer->capacity() - partial_size),
                                boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), partial_size,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
      }

      // post job with message buffer
      _session->strand().get_io_service().post(boost::bind<void>(_message_handler, _session,
                                                           buf->get_shared_buffer(cur, header.message_size)));

      cur += header.message_size;
    }

    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), 0,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  } else {
    _session->stop();
  }
}

void TcpConnection::write_handler(WriteHandler callback, const boost::system::error_code &error) {
  if (!error)
    callback();
  else {
    if (_session != nullptr) _session->stop();
    else close();
  }
}

void TcpConnection::write(BufferPtr buf, size_t size, WriteHandler callback) {
  // check to see if current number of pending messages is above allowed limit
  if (++_pending_messages > _config.max_pending_messages()) {
    close();
    return;
  }

  boost::asio::async_write(_socket, boost::asio::buffer(buf->data(), size),
                           boost::bind(&TcpConnection::write_handler, share_this<TcpConnection>(), callback,
                                       boost::asio::placeholders::error));
}

void TcpConnection::start() throw() {
  if (_session == nullptr) {
    close();
    throw std::runtime_error("Error: connection started with no session");
  }

  name();

  _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                          boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), 0,
                                      boost::asio::placeholders::error, 0));
}

tcp_socket &TcpConnection::socket() { return _socket; }

//////////////////////////////////////
// TcpServerConnection
//////////////////////////////////////
TcpServerConnection::TcpServerConnection(const App &app, const Server::Config &config)
    : TcpConnection(app, Config(config.message_handler())) {
  std::cout << "TcpServerConnection()" << std::endl;
  _path = std::make_shared<Buffer>(config.path());
}

void TcpServerConnection::connect() {
  start_handshake();
}

void TcpServerConnection::start_handshake() {
  // start timeout timer with handshake timeout specified in config
  _deadline.expires_from_now(boost::posix_time::milliseconds(_config.handshake_timout()));
  _deadline.async_wait(boost::bind(&TcpServerConnection::timeout, share_this<TcpServerConnection>(),
                                   boost::asio::placeholders::error));

  // start listening for f0
  _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                          boost::bind(&TcpServerConnection::f0_received, share_this<TcpServerConnection>(),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));

  // prepare and send f1 then make sure it was successful [success_or_close(...)]
  size_t f1_size = load_f1(*_write_buffer);
  boost::asio::async_write(_socket, boost::asio::buffer(_write_buffer->data(), f1_size),
                           boost::bind(&TcpServerConnection::success_or_close, share_this<TcpServerConnection>(),
                                       boost::asio::placeholders::error));
}

void TcpServerConnection::f0_received(const boost::system::error_code &error, size_t bytes_transferred) {
  // reset timeout
  _deadline.expires_from_now(boost::posix_time::milliseconds(_config.handshake_timout()));
  _deadline.async_wait(boost::bind(&TcpServerConnection::timeout, share_this<TcpServerConnection>(),
                                   boost::asio::placeholders::error));

  if (!error && parse_f0(bytes_transferred)) {
    // compute shared secret
    compute_secret();

    // read and goto -> f2_received()
    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpServerConnection::f2_received, share_this<TcpServerConnection>(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  } else {
    close();
  }
}

void TcpServerConnection::f2_received(const boost::system::error_code &error, size_t bytes_transferred) {
  // start dsa standard 1 minute timeout
  reset_standard_deadline_timer();

  if (!error && parse_f2(bytes_transferred)) {
    // setup session now that client session id has been parsed
    if (auto server = _server.lock()) {
      std::string session_id = _session_id->to_string();
      _session = server->session_manager()->get_session(_security_context.dsid(), session_id);
      if (_session == nullptr) _session = server->session_manager()->create_session(_security_context.dsid());
    } else {
      // if server no longer exists, connection needs to shutdown
      return;
    }

    _session_id = _session->session_id();

    // send f3 then start session
    send_f3();
    _session->set_connection(shared_from_this());
    _session->start();
  } else {
    close();
  }
}

void TcpServerConnection::send_f3() {
  // send f3
  size_t f3_size = load_f3(*_read_buffer);
  boost::asio::async_write(_socket, boost::asio::buffer(_read_buffer->data(), f3_size),
                           boost::bind(&TcpServerConnection::success_or_close, shared_from_this(),
                                       boost::asio::placeholders::error));
}

//////////////////////////////////
// TcpClientConnection
//////////////////////////////////
TcpClientConnection::TcpClientConnection(std::shared_ptr<App> app)
    : TcpConnection(*app, Config()), GracefullyClosable(app) { std::cout << "TcpClientConnection()\n"; }

TcpClientConnection::TcpClientConnection(std::shared_ptr<App> app, const Config &config)
    : TcpConnection(*app, config), GracefullyClosable(app) { std::cout << "TcpClientConnection()\n"; }

void TcpClientConnection::connect() {
  // register this client with app for graceful closing
  register_this();

  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_app->io_service());
  tcp::resolver::query query(_config.host(), std::to_string(_config.port()));
  tcp::endpoint endpoint = *resolver.resolve(query);
  _socket.async_connect(*resolver.resolve(query),
                        boost::bind(&TcpClientConnection::start_handshake, Connection::share_this<TcpClientConnection>(),
                                    boost::asio::placeholders::error));
}

void TcpClientConnection::start_handshake(const boost::system::error_code &error) {
  if (error != nullptr) {
    close();
    throw std::runtime_error("Couldn't connect to specified host");
  }

  // start timeout timer
  _deadline.expires_from_now(boost::posix_time::milliseconds(_config.handshake_timout()));
  _deadline.async_wait(boost::bind(&TcpClientConnection::timeout, Connection::share_this<TcpClientConnection>(),
                                   boost::asio::placeholders::error));

  _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                          boost::bind(&TcpClientConnection::f1_received,
                                      Connection::share_this<TcpClientConnection>(),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));

  size_t f0_size = load_f0(*_write_buffer);
  boost::asio::async_write(_socket, boost::asio::buffer(_write_buffer->data(), f0_size),
                           boost::bind(&TcpClientConnection::success_or_close,
                                       Connection::share_this<TcpClientConnection>(),
                                       boost::asio::placeholders::error));
}

void TcpClientConnection::f1_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && parse_f1(bytes_transferred)) {
    // cancel timer before timeout
    _deadline.expires_from_now(boost::posix_time::milliseconds(_config.handshake_timout()));

    // server should be parsing f0 and waiting for f2 at this point
    // so we can compute the shared secret synchronously
    compute_secret();
    size_t f2_size = load_f2(*_write_buffer);
    boost::asio::async_write(_socket, boost::asio::buffer(_write_buffer->data(), f2_size),
                             boost::bind(&TcpClientConnection::success_or_close,
                                         Connection::share_this<TcpClientConnection>(),
                                         boost::asio::placeholders::error));

    // restart timeout timer
    _deadline.async_wait(boost::bind(&TcpClientConnection::timeout,
                                     Connection::share_this<TcpClientConnection>(),
                                     boost::asio::placeholders::error));

    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpClientConnection::f3_received,
                                        Connection::share_this<TcpClientConnection>(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  } else {
    close();
  }
}

void TcpClientConnection::f3_received(const boost::system::error_code &error, size_t bytes_transferred) {
  // start standard dsa 1 minute timeout
  reset_standard_deadline_timer();

  if (!error && parse_f3(bytes_transferred)) {
    uint8_t *auth = _auth->data(), *other_auth = _other_auth->data();
    for (size_t i = 0; i < AuthLength; ++i)
      if (auth[i] != other_auth[i]) return;

    // create new session object and start
    _session = std::make_shared<Session>(_session_id, Connection::shared_from_this());
    _session->start();

    _session->stop();
  } else {
    close();
  }
}

}  // namespace dsa