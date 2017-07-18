#include <message/static_header.h>
#include "connection.h"

namespace dsa {

Connection::Connection(boost::asio::io_service &io_service) : io_service(io_service), _buffer(new Buffer()) {}

void Connection::set_read_handler(ReadCallback callback) {
  read_handler = callback;
}

void Connection::handle_read(Buffer::MessageBuffer buf) {
  read_handler(buf);
}

// Handshake parse functions
bool Connection::parse_f0(size_t size) {
  if (size < min_f0_length)
    return false;

  const uint8_t *data = _buffer->data();

  StaticHeader header(data);
  if (header.message_size() != size ||
      header.header_size() != static_header_size||
      header.type() != 0xf0 ||
      header.request_id() != 0)
    return false;

  uint32_t cur = static_header_size;
  uint8_t dsid_length;

  std::memcpy(&_dsa_version_major, &data[cur], sizeof(_dsa_version_major));
  cur += sizeof(_dsa_version_major);
  std::memcpy(&_dsa_version_minor, &data[cur], sizeof(_dsa_version_minor));
  cur += sizeof(_dsa_version_minor);
  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length > size) return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);
  _other_public_key = std::make_shared<Buffer>()

  return true;
}

bool Connection::parse_f1(size_t bytes_transferred) {
  return true;
}

bool Connection::parse_f2(size_t bytes_transferred) {
  return true;
}

bool Connection::parse_f3(size_t bytes_transferred) {
  return true;
}

// Handshake load functions
size_t Connection::load_f0(Buffer &buf) {
  return 0;
}

size_t Connection::load_f1(Buffer &buf) {
  return 0;
}

size_t Connection::load_f2(Buffer &buf) {
  return 0;
}

size_t Connection::load_f3(Buffer &buf) {
  return 0;
}

}  // namespace dsa