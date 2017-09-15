#ifndef DSA_SDK_INCOMING_LIST_STREAM_H
#define DSA_SDK_INCOMING_LIST_STREAM_H

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {

class ListResponseMessage;

class IncomingListStream : public MessageCacheStream {
 public:
  typedef std::function<void(ref_<const ListResponseMessage>&&,
                             IncomingListStream&)>
      Callback;

 protected:
  Callback _callback;

 public:
  explicit IncomingListStream(ref_<Session>&& session, const Path& path,
                              uint32_t rid, Callback&& callback);
  void receive_message(MessageCRef&& msg) override;

  void list();
};
}

#endif  // DSA_SDK_INCOMING_LIST_STREAM_H
