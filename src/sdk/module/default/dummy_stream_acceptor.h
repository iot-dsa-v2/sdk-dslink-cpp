#ifndef DSA_SDK_MODULE_DUMMY_STREAM_ACCEPTOR_H
#define DSA_SDK_MODULE_DUMMY_STREAM_ACCEPTOR_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../outgoing_stream_acceptor.h"

namespace dsa {
class DummyStreamAcceptor : public OutgoingStreamAcceptor {
public:
  void add(ref_<OutgoingSubscribeStream> &&stream) override;
  void add(ref_<OutgoingListStream> &&stream)  override;
  void add(ref_<OutgoingInvokeStream> &&stream)  override;
  void add(ref_<OutgoingSetStream> &&stream)  override;
};
}

#endif  // DSA_SDK_MODULE_DUMMY_STREAM_ACCEPTOR_H
