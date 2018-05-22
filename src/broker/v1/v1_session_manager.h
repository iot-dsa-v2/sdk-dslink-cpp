#ifndef DSA_SDK_V1_SESSION_H
#define DSA_SDK_V1_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "core/shared_strand_ref.h"
#include "v1_session.h"

namespace dsa {

class NodeStateManager;
class V1ServerConnection;
class Websocket;

class V1SessionManager final : public SharedStrandPtr<V1SessionManager> {
  friend class V1ServerConnection;

  shared_ptr_<SharedRef<NodeStateManager>> _strand_state_manager;

 protected:
  void destroy_impl() final;

 public:
  void post_in_strand(std::function<void()>&& callback) override;

  V1SessionManager(const LinkStrandRef& strand,
                   NodeStateManager& state_manager);
  ~V1SessionManager();

  string_ on_conn(const string_& dsid, const string_& token,
                  const string_& body);
  void on_ws(std::unique_ptr<Websocket>&& ws);
};
}  // namespace dsa

#endif  // DSA_SDK_V1_SESSION_H
