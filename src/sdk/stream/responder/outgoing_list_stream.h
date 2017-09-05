#ifndef DSA_SDK_OUTGOING_LIST_STREAM_H
#define DSA_SDK_OUTGOING_LIST_STREAM_H

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {
class OutgoingListStream : public MessageRefedStream {
protected:
  ListOptions _option;

public:
  OutgoingListStream(ref_<Session> &&session, const Path &path,
    uint32_t rid, ListOptions &&options);

};
}




#endif //DSA_SDK_OUTGOING_LIST_STREAM_H
