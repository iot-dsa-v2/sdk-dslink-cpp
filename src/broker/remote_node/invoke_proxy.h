#ifndef DSA_BROKER_REMOTE_INVOKE_PROXY_H
#define DSA_BROKER_REMOTE_INVOKE_PROXY_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"

namespace dsa {
class IncomingInvokeStream;
class OutgoingInvokeStream;
class RemoteNode;

class RemoteInvokeProxy final : public DestroyableRef<RemoteInvokeProxy> {
 public:
  RemoteInvokeProxy();
  RemoteInvokeProxy(ref_<OutgoingInvokeStream>&& stream, ref_<RemoteNode> node);

 protected:
  ref_<RemoteNode> _node;
  ref_<IncomingInvokeStream> _in_stream;
  ref_<OutgoingInvokeStream> _out_stream;

  void destroy_impl() final;
};
}
#endif  // DSA_BROKER_REMOTE_INVOKE_PROXY_H
