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
  std::unordered_map<string_, VarBytesRef> _override_metas;
  void send_all_override_metas();
 public:
  RemoteRootNode(LinkStrandRef &&strand, ref_<Session> &&session);
  ~RemoteRootNode() override;

  // updated the status of connection changes, not used yet
  // void on_session(Session &session, const shared_ptr_<Connection> &connection);

  VarBytesRef &get_summary() override;

  void set_override_meta(const string_ &field, Var &&v);

 protected:
  void on_list(BaseOutgoingListStream &stream, bool first_request) override;

};
}
#endif  // DSA_BROKER_REMOTE_ROOT_NODE_H
