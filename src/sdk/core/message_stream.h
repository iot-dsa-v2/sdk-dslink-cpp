#ifndef DSA_SDK_NETWORK_STREAM_H
#define DSA_SDK_NETWORK_STREAM_H

#include <boost/asio/strand.hpp>
#include <map>

#include "message/base_message.h"
#include "util/enable_intrusive.h"

namespace dsa {

class Session;
class MessageStream : public EnableIntrusive<MessageStream> {
 protected:
  typedef std::map<uint32_t, intrusive_ptr_<MessageStream> > *_stream_container;
  boost::asio::io_service::strand &_strand;
  _stream_container _container;

 public:
  struct StreamInfo {
    uint32_t rid;
    size_t unique_id;
    _stream_container container;
  };

  const uint32_t _request_id;
  const size_t _unique_id;

  MessageStream(intrusive_ptr_<Session> session, _stream_container container,
                uint32_t request_id, size_t unique_id);
  virtual ~MessageStream() = default;

  virtual size_t get_next_message_size() = 0;
  virtual const Message &get_next_message() = 0;
  StreamInfo get_info() { return {_request_id, _unique_id, _container}; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_STREAM_H
