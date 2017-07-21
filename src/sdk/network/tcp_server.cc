#include <boost/bind.hpp>
#include "tcp_server.h"

#include "tcp_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

TcpServer::TcpServer(const App &app, const Config &config)
    : Server(app), _config(config),
      _acceptor(new tcp::acceptor(app.io_service(), tcp::endpoint(tcp::v4(), config.port()))) {}

void TcpServer::start() {
  // start taking connections
  _new_connection = std::make_shared<TcpServerConnection>(_app, _config);

  _acceptor->async_accept(_new_connection->socket(),
                          boost::bind(&TcpServer::accept_loop,
                                      share_this<TcpServer>(),
                                      boost::asio::placeholders::error));
}

void TcpServer::stop() {
  _acceptor.reset();
  Server::stop();
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  if (!error) {
    _new_connection->set_server(share_this<TcpServer>());
    _new_connection->connect();
    _new_connection.reset(new TcpServerConnection(_app, _config));
    _acceptor->async_accept(_new_connection->socket(),
                            boost::bind(&TcpServer::accept_loop,
                                        share_this<TcpServer>(),
                                        boost::asio::placeholders::error));
  }
}

SessionPtr TcpServer::get_session(const std::string &session_id) {
  boost::upgrade_lock<boost::shared_mutex> lock(_sessions_key);
  if (_sessions.count(session_id) != 0)
    return _sessions.at(session_id);
  return nullptr;
}

SessionPtr TcpServer::create_session() {
  std::string session_id = get_new_session_id();
  auto session = std::make_shared<Session>(session_id);

  {
    boost::unique_lock<boost::shared_mutex> lock(_sessions_key);
    _sessions[session_id] = session;
  }

  return std::move(session);
}

std::string TcpServer::get_new_session_id() {
  Hash hash("sha256");
  hash.update(*gen_salt(32));
  return std::move(std::to_string(_session_count++) + hash.digest_base64());
}

}  // namespace dsa