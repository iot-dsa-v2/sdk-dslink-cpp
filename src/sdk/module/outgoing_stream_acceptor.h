#ifndef DSA_SDK_OUTGOING_STREAM_ACCEPTER_H
#define DSA_SDK_OUTGOING_STREAM_ACCEPTER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"

namespace dsa {
class OutgoingSubscribeStream;
class OutgoingListStream;
class OutgoingInvokeStream;
class OutgoingSetStream;

class OutgoingStreamAcceptor : public DestroyableRef<OutgoingStreamAcceptor> {
 public:
  virtual ~OutgoingStreamAcceptor() = default;

  virtual void add(ref_<OutgoingSubscribeStream> &&stream) = 0;
  virtual void add(ref_<OutgoingListStream> &&stream) = 0;
  virtual void add(ref_<OutgoingInvokeStream> &&stream) = 0;
  virtual void add(ref_<OutgoingSetStream> &&stream) = 0;
};
}

#endif  // DSA_SDK_OUTGOING_STREAM_ACCEPTER_H
