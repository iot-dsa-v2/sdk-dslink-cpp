#ifndef DSA_SDK_STANDARD_PROFILES_H
#define DSA_SDK_STANDARD_PROFILES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class PubRoot : public NodeModel {
  void init(const string_& profile);

 public:
  PubRoot(LinkStrandRef&& strand, const string_& profile);
  ~PubRoot() override;

  ref_<NodeModel> add(const string_& path, ref_<NodeModel>&& node);
};
}

#endif  // DSA_SDK_STANDARD_PROFILES_H
