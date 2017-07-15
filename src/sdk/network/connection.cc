#include "connection.h"

namespace dsa {

void Connection::set_read_handler(ReadCallback callback) {
  read_handler = callback;
}

void Connection::handle_read(Buffer::MessageBuffer buf) {
  read_handler(buf);
}

}  // namespace dsa