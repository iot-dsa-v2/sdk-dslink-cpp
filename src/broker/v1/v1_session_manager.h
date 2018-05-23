#ifndef DSA_SDK_V1_SESSION_H
#define DSA_SDK_V1_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <memory>
#include "core/link_strand.h"
#include "core/shared_strand_ref.h"
#include "v1_session.h"

namespace dsa {

class NodeStateManager;
class V1ServerConnection;
class Websocket;

class V1SessionManager final : public DestroyableRef<V1SessionManager> {
  friend class V1ServerConnection;

  LinkStrandRef _strand;
  ref_<NodeStateManager> _state_manager;

  shared_ptr_<SharedRef<V1SessionManager>> _shared_ptr;

 protected:
  void destroy_impl() final;

 public:
  V1SessionManager(const LinkStrandRef& strand,
                   NodeStateManager& state_manager);
  ~V1SessionManager();

  shared_ptr_<SharedRef<V1SessionManager>>& share_this() {
    return _shared_ptr;
  };

  void on_conn(const string_& dsid, const string_& token, const string_& body,
               std::function<void(const string_&)>&&);
  void on_ws(shared_ptr_<Websocket>&& ws);
};
}  // namespace dsa

#endif  // DSA_SDK_V1_SESSION_H
