#ifndef DSA_SDK_STANDARD_PROFILES_H
#define DSA_SDK_STANDARD_PROFILES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/invoke_node_model.h"
#include "responder/node_model.h"

namespace dsa {

class PubRoot : public NodeModel {
  void load_standard_profiles(const string_& profile);

  std::unordered_map<string_, ref_<NodeModel>> _standard_profiles;

 protected:
  void initialize() override;
  void add_standard_node(const string_& path, ref_<NodeModel> model);

  void destroy_impl() override;

 public:
  PubRoot(const LinkStrandRef &strand, const string_& profile);
  ~PubRoot() override;

  ref_<NodeModel>& get_standard_profile(const string_& path);

  ref_<NodeModel> add(const string_& path, ref_<NodeModel>&& node);

  void register_standard_profile_function(
      const string_& path, SimpleInvokeNode::FullCallback&& callback);
};
}  // namespace dsa

#endif  // DSA_SDK_STANDARD_PROFILES_H
