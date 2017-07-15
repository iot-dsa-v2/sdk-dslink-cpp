#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <boost/function.hpp>
#include <boost/asio.hpp>

#include "dsa/util.h"

typedef boost::function0<void> WriteCallback;
typedef boost::function1<void, dsa::Buffer::MessageBuffer> ReadCallback;

namespace dsa {

/**
 * handshake logic
 * split and join binary data into message frame
 */
class Connection : public std::enable_shared_from_this<Connection> {
 private:
  ReadCallback read_handler;
  void handle_read(Buffer::MessageBuffer buf);
 public:
  void set_read_handler(ReadCallback callback);
  virtual void write(BufferPtr buf, WriteCallback callback) = 0;
  virtual void close() = 0;
  virtual void start() = 0;
};
}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
