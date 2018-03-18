#include "dsa_common.h"

#include "pub_root.h"

#include "module/logger.h"
#include "responder/invoke_node_model.h"

#include "standard_profile_json.h"

namespace dsa {

PubRoot::PubRoot(LinkStrandRef &&strand, const string_ &profile)
    : NodeModel(std::move(strand)) {
  if (profile.empty()) {
    init(standard_profile);
  } else {
    init(profile);
  }
}
PubRoot::~PubRoot() = default;

static void load_profile_model(NodeModel *node, VarMap &map) {
  node->load(map);
  for (auto &it : map) {
    if (it.first.empty()) continue;
    switch (it.first[0]) {
      case '$':
      case '@':
      case '?':
        // already handled in node->load
        break;
      default:
        if (it.second.is_map()) {
          VarMap &child_map = it.second.get_map();
          ref_<NodeModel> child;
          if (child_map.count("$invokable") > 0) {
            // child action
            child.reset(
                new SimpleInvokeNode(node->get_strand()->get_ref(),
                                     SimpleInvokeNode::FullCallback(nullptr)));
          } else {
            child.reset(new NodeModel(node->get_strand()->get_ref()));
          }
          load_profile_model(child.get(), child_map);
          node->add_list_child(it.first, std::move(child));
        }
    }
  }
}

void PubRoot::init(const string_ &profile) {
  auto v = Var::from_json(profile);
  if (v.is_map()) {
    load_profile_model(this, v.get_map());
  }
}

ref_<NodeModel> PubRoot::add(const string_ &path_str, ref_<NodeModel> &&node) {
  Path path(path_str);
  if (path.is_invalid()) {
    LOG_FATAL(__FILENAME__, LOG << "invalid pub node path: " << path_str);
  }

  NodeModel *pub_parent = this;
  while (!path.is_last()) {
    ModelRef child = pub_parent->get_child(path.current_name());
    if (child == nullptr) {
      pub_parent = static_cast<NodeModel *>(
          pub_parent
              ->add_list_child(path.current_name(),
                               make_ref_<NodeModel>(_strand->get_ref()))
              .get());
    } else {
      pub_parent = dynamic_cast<NodeModel *>(child.get());
      if (pub_parent == nullptr) {
        LOG_FATAL(__FILENAME__, LOG << "failed to add pub node: " << path_str);
      }
    }
    path = path.next();
  }
  return std::move(
      pub_parent->add_list_child(path.current_name(), std::move(node)));
}
}
