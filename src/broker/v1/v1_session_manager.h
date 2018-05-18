#ifndef DSA_SDK_V1_SESSION_H
#define DSA_SDK_V1_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "v1_session.h"

namespace dsa {

class NodeStateManager;

class V1SessionManager final : public DestroyableRef<V1SessionManager> {
  LinkStrandRef _strand;

  ref_<NodeStateManager> _state_manager;

 protected:
  void destroy_impl() final;

 public:
  V1SessionManager(const LinkStrandRef &strand,
                   NodeStateManager &state_manager);
  ~V1SessionManager();
};
}  // namespace dsa

#endif  // DSA_SDK_V1_SESSION_H
