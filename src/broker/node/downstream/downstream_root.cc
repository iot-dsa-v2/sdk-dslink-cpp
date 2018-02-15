#include "dsa_common.h"

#include "downstream_root.h"

#include "../../remote_node/remote_root_node.h"
#include "util/client_info.h"
#include "core/session.h"
#include "util/string.h"

namespace dsa {

static const string_ DOWNSRTEAM_PATH = "downstream/";
static const size_t DOWNSTREAM_PATH_LEN = DOWNSRTEAM_PATH.size();

DownstreamRoot::DownstreamRoot(LinkStrandRef &&strand)
    : NodeModel(std::move(strand)) {}

ref_<RemoteRootNode> DownstreamRoot::get_root_for_client(const ClientInfo &info,
                                                         Session &session) {
  // client responder path must start with downstream/
  if (str_starts_with(info.responder_path, DOWNSRTEAM_PATH)) {
    string_ name = info.responder_path.substr(DOWNSTREAM_PATH_LEN);
    // check if name is valid path name
    if (!PathData::invalid_name(name)) {
      auto child_model = get_child(name);
      if (child_model == nullptr) {
        // create new downstream root node
        auto new_root =
            make_ref_<RemoteRootNode>(_strand->get_ref(), session.get_ref());
        new_root->set_override_meta("$$dsid", Var(info.dsid));
        child_model = std::move(new_root);
        add_list_child(name, child_model->get_ref());
      }
      return std::move(child_model);
    }
  }
  // invalid input, return a nullptr
  return ref_<RemoteRootNode>();
}
}