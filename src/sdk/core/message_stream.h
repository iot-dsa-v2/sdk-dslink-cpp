#ifndef DSA_SDK_NETWORK_STREAM_H
#define DSA_SDK_NETWORK_STREAM_H

#include <boost/asio/strand.hpp>


#include "message/base_message.h"

namespace dsa {

class Session;

class MessageStream {
 protected:
  boost::asio::io_service::strand &_strand;

 public:
  const uint32_t _request_id;
  const size_t _unique_id;

  MessageStream(const intrusive_ptr_<Session> &session, uint32_t request_id, size_t unique_id);


  virtual size_t get_next_message_size() = 0;
  virtual const Message& get_next_message() = 0;
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_STREAM_H
