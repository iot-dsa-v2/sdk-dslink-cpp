#ifndef DSA_BROKER_REMOTE_SET_PROXY_H
#define DSA_BROKER_REMOTE_SET_PROXY_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"

namespace dsa {
class IncomingSetStream;
class OutgoingSetStream;
class RemoteNode;

class RemoteSetProxy final : public DestroyableRef<RemoteSetProxy> {
 public:
  RemoteSetProxy();
  RemoteSetProxy(ref_<OutgoingSetStream>&& stream, ref_<RemoteNode> node);

 protected:
  ref_<RemoteNode> _node;
  ref_<IncomingSetStream> _in_stream;
  ref_<OutgoingSetStream> _out_stream;

  void destroy_impl() final;
};
}
#endif  // DSA_BROKER_REMOTE_SET_PROXY_H
