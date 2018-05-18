#ifndef DSA_SDK_V1_CONNECTION_H
#define DSA_SDK_V1_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "core/shared_strand_ref.h"
#include "util/enable_shared.h"

namespace dsa {
class V1ServerConnection : public SharedStrandPtr<V1ServerConnection> {
 protected:
  SharedLinkStrandRef _shared_strand;

  void destroy_impl() override;

 public:
  void post_in_strand(std::function<void()> &&callback) override;

  V1ServerConnection(const SharedLinkStrandRef &strand);
};
}  // namespace dsa

#endif  // DSA_SDK_V1_CONNECTION_H
