#include <dsa_common.h>

#include <queue>

#include <boost/asio.hpp>

#include "core/connection.h"
#include "core/message_stream.h"

namespace dsa {
// has access to strand only
class RequiresStrand {
 protected:
  shared_ptr_<boost::asio::io_service::strand> _strand;
  RequiresStrand() : _strand(nullptr) {}
  explicit RequiresStrand(shared_ptr_<boost::asio::io_service::strand> &&strand) : _strand(strand) {}
  virtual ~RequiresStrand() = default;
};

// has access to connection only
class RequiresConnection {
 protected:
  shared_ptr_<Connection> _connection;
  RequiresConnection() : _connection(nullptr) {}
  explicit RequiresConnection(shared_ptr_<Connection> &&connection) : _connection(connection) {}
  virtual ~RequiresConnection() = default;
};

// has access to strand, connection, private ready_streams,
// provides interface for posting new streams when ready, handles write loop
class StreamToConnectionWriter : public RequiresStrand, public RequiresConnection  {
 private:
  std::queue< intrusive_ptr_<MessageStream> > _ready_streams;
  std::atomic_bool is_writing{false};

 protected:
  StreamToConnectionWriter(shared_ptr_<boost::asio::io_service::strand> &&strand,
                           shared_ptr_<Connection> &&connection)
      : RequiresStrand(std::move(strand)), RequiresConnection(std::move(connection)) {}

  static void write_messages_loop(intrusive_ptr_<StreamToConnectionWriter> &&sthis) {
    if (sthis->_ready_streams.empty()) return;

    auto buf = make_intrusive_<Buffer>();
    // pull from ready streams and write messages to connection
    // ...
    size_t size = buf->size();
    sthis->_connection->write(std::move(buf), size, [&strand = *sthis->_strand, callback = [sthis = std::move(sthis)](){
      write_messages_loop(std::move(sthis));
    }]() mutable {
      strand.dispatch(callback);
    });
  }

  void add_ready_stream(intrusive_ptr_<MessageStream> &&stream) {
    _strand->post([stream = std::move(stream)]() mutable {
      _ready_streams.push(std::move(stream));
    });
  }
};

// has access to incoming streams, strand, and connection
// contains all requester logic
class ExampleRequester : virtual public StreamToConnectionWriter {
 protected:
  ExampleRequester() : StreamToConnectionWriter(nullptr, nullptr) {}

  std::map< uint32_t, intrusive_ptr_<MessageStream> > _incoming_streams;
};

// has access to outgoing_streams, strand, and connection
// contains all responder logic
class ExampleResponder : virtual public StreamToConnectionWriter {
 protected:
  ExampleResponder() : StreamToConnectionWriter(nullptr, nullptr) {}

  std::map< uint32_t, intrusive_ptr_<MessageStream> > _outgoing_streams;
};

// has access to incoming_streams, outgoing_streams, strand, and connection
class ExampleSession : public ExampleRequester, public ExampleResponder {
 public:
  ExampleSession(shared_ptr_<boost::asio::io_service::strand> &&strand,
          shared_ptr_<Connection> &&connection)
      : StreamToConnectionWriter(std::move(strand), std::move(connection)) {}
};

}  // namespace dsa
