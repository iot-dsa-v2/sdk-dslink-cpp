#ifndef DSA_BROKER_REMOTE_ROOT_NODE_H
#define DSA_BROKER_REMOTE_ROOT_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "remote_node.h"

namespace dsa {

class Session;
class Connection;

class RemoteRootNode : public RemoteNode {
  ref_<Session> _remote_session;

  std::unordered_map<string_, BytesRef> _override_metas;

 public:
  RemoteRootNode(LinkStrandRef &&strand, ref_<Session> &&session);
  ~RemoteRootNode() override;

  // updated the status of connection changes
  void on_session(Session &session, const shared_ptr_<Connection> &connection);

  VarBytesRef &get_summary() override;

 protected:
  void on_list(BaseOutgoingListStream &stream, bool first_request) override;
};
}
#endif  // DSA_BROKER_REMOTE_ROOT_NODE_H
