#include "dsa_common.h"

#include "pub_root.h"

#include "message/response/set_response_message.h"
#include "module/logger.h"
#include "responder/invoke_node_model.h"
#include "standard_profile_json.h"
#include "stream/responder/outgoing_set_stream.h"

namespace dsa {

class PubValueNode : public NodeModel {
 public:
  PubValueNode(const LinkStrandRef &strand) : NodeModel(strand) {}

 protected:
  // pub node should not be subscribed directly
  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
    if (first_request) {
      auto response = make_ref_<SubscribeResponseMessage>();
      response->set_status(Status::NOT_SUPPORTED);
      set_subscribe_response(std::move(response));
    }
  }
  // pub node should not be set directly
  void set(ref_<OutgoingSetStream> &&stream) override {
    stream->close(Status::NOT_SUPPORTED);
  }
};

PubRoot::PubRoot(const LinkStrandRef &strand, const string_ &profile)
    : NodeModel(strand) {
  if (profile.empty()) {
    load_standard_profiles(standard_profile);
  } else {
    load_standard_profiles(profile);
  }
}
PubRoot::~PubRoot() = default;

void PubRoot::destroy_impl() {
  _standard_profiles.clear();
  NodeModel::destroy_impl();
}
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
          } else if (child_map.count("$type") > 0) {
            // child value
            child.reset(new PubValueNode(node->get_strand()->get_ref()));
          } else {
            child.reset(new NodeModel(node->get_strand()->get_ref()));
          }
          load_profile_model(child.get(), child_map);
          node->add_list_child(it.first, std::move(child));
        }
    }
  }
}

void PubRoot::load_standard_profiles(const string_ &profile) {
  auto v = Var::from_json(profile);
  if (v.is_map()) {
    load_profile_model(this, v.get_map());
  }
}

ref_<NodeModel> &PubRoot::get_standard_profile(const string_ &path) {
  auto search = _standard_profiles.find(path);
  if (search == _standard_profiles.end()) {
    LOG_FATAL(__FILENAME__, LOG << "standard profile doesn't exist: " << path);
  }
  return search->second;
}

void PubRoot::add_standard_node(const string_ &path, ref_<NodeModel> model) {
  if (_standard_profiles.count(path) == 0) {
    _standard_profiles[path] = model;
  }

  for (auto it : model->get_list_children()) {
    auto *model = dynamic_cast<NodeModel *>(it.second.get());
    // standard profile group start with number
    if (model != nullptr) {
      add_standard_node(path + "/" + it.first, ref_<NodeModel>(model));
    }
  }
}

void PubRoot::initialize() {
  NodeModel::initialize();

  // add standard profile to the cached map
  // need a sorted iteration
  std::map<string_, ref_<NodeModel>> standard_groups;
  for (auto it : _list_children) {
    char first_c = it.first[0];
    auto *model = dynamic_cast<NodeModel *>(it.second.get());
    // standard profile group start with number
    if (first_c >= '2' && first_c <= '9' && model != nullptr) {
      standard_groups[it.first] = ref_<NodeModel>(model);
    }
  }
  // iterate the map with sorted key
  for (auto it = standard_groups.rbegin(); it != standard_groups.rend(); ++it) {
    // add all children node to standard profile
    for (auto child_it : it->second->get_list_children()) {
      auto *model = dynamic_cast<NodeModel *>(child_it.second.get());
      // standard profile group start with number
      if (model != nullptr) {
        add_standard_node(child_it.first, ref_<NodeModel>(model));
      }
    }
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
                               make_ref_<NodeModel>(_strand))
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

void PubRoot::register_standard_profile_function(
    const string_ &path, SimpleInvokeNode::FullCallback &&callback) {
  auto search = _standard_profiles.find(path);
  if (search == _standard_profiles.end()) {
    LOG_FATAL(__FILENAME__,
              LOG << "not able to register standard function " << path);
  }
  auto *invoke_node = dynamic_cast<SimpleInvokeNode *>(search->second.get());
  if (invoke_node == nullptr) {
    LOG_FATAL(__FILENAME__, LOG << "not able to register standard function "
                                << "not an action node " << path);
  }
  invoke_node->set_callback(std::move(callback));
}

}  // namespace dsa
