#ifndef DSA_SDK_BROKER_AUTHORIZER_H
#define DSA_SDK_BROKER_AUTHORIZER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "module/authorizer.h"

namespace dsa {
class DsBroker;

class BrokerAuthorizer : public Authorizer {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;
  void set_strand(LinkStrandRef strand) { _strand = std::move(strand); }

 public:
  BrokerAuthorizer() = default;
  void check_permission(const string_& dsid, const string_& permission_token,
                        MessageType method, const Path& path,
                        CheckPermissionCallback&& callback) override;
};
}

#endif  // DSA_SDK_BROKER_AUTHORIZER_H
