#ifndef DSA_SDK_NETWORK_STREAM_H
#define DSA_SDK_NETWORK_STREAM_H

#include <boost/asio/strand.hpp>

#include "dsa/util.h"
#include "message/base_message.h"
#include "session.h"

namespace dsa {
class MessageStream : public InheritableEnableShared<MessageStream> {
 protected:
  boost::asio::io_service::strand &_strand;

 public:
  const uint32_t _request_id;
  const size_t _unique_id;

  MessageStream(const intrusive_ptr_<Session> &session, uint32_t request_id, size_t unique_id)
      : _strand(session->strand()), _request_id(request_id), _unique_id(unique_id) {}

  virtual size_t get_next_message_size() = 0;
  virtual const Message& get_next_message() = 0;
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_STREAM_H
