#ifndef DSA_SDK_V1_SESSION_MANAGER_H
#define DSA_SDK_V1_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <memory>
#include <unordered_map>
#include "core/link_strand.h"
#include "core/shared_strand_ref.h"

namespace dsa {

class NodeStateManager;
class V1ServerConnection;
class Websocket;
class V1Session;

class V1SessionManager final : public DestroyableRef<V1SessionManager> {
  friend class V1ServerConnection;

  string_ _dsid;
  string_ _publick_key_b64;
  LinkStrandRef _strand;
  ref_<NodeStateManager> _state_manager;

  shared_ptr_<SharedRef<V1SessionManager>> _shared_ptr;

  std::unordered_map<string_, ref_<V1Session>> _sessions;

 protected:
  void destroy_impl() final;

 public:
  V1SessionManager(const LinkStrandRef& strand, NodeStateManager& state_manager,
                   const string_& dsid);
  ~V1SessionManager();

  shared_ptr_<SharedRef<V1SessionManager>>& share_this() {
    return _shared_ptr;
  };

  void on_conn(const string_& dsid, const string_& token, const string_& body,
               std::function<void(const string_&)>&&);
  void on_ws(shared_ptr_<Websocket>&& ws, const string_& dsid,
             const string_& auth);
};
}  // namespace dsa

#endif  // DSA_SDK_V1_SESSION_MANAGER_H
