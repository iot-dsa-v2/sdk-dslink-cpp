#include "dsa_common.h"

#include "token_nodes.h"

#include "module/storage.h"
#include "responder/invoke_node_model.h"
#include "responder/node_state.h"
#include "util/string.h"

namespace dsa {
TokensRoot::TokensRoot(const LinkStrandRef &strand)
    : NodeModel(strand),
      _storage(_strand->storage().get_strand_bucket("Tokens", _strand)) {
  add_list_child(
      "Add", make_ref_<SimpleInvokeNode>(
                 strand, [ this, keepref = get_ref() ](Var && v)->Var {
                   if (v.is_map()) {
                     string_ role = v["Role"].to_string();
                     string_ time_range = v["Time_Range"].to_string();
                     int64_t count = v["Count"].to_int(-1);
                     int64_t max_session = v["Max_Session"].to_int(1);
                     bool managed = v["Manager"].to_bool();

                     string_ token_name = generate_random_string(16);
                     while (_list_children.count(token_name) > 0) {
                       token_name = generate_random_string(16);
                     }
                     string_ token = token_name + generate_random_string(32);
                   }
                   return Var(Status::INVALID_PARAMETER);
                 },
                 PermissionLevel::CONFIG));
}
TokensRoot::~TokensRoot() = default;

TokenNode::TokenNode(const LinkStrandRef &strand, ref_<TokensRoot> &&parent,
                     ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)), _parent(std::move(parent)) {}

void TokenNode::remove_all_clients() {}
void TokenNode::regenerate() {
  // only change the rest
  _token = _token.substr(0, 16) + generate_random_string(32);
  save_token();
}

void TokenNode::save_token() const {
  save(*_parent->_storage, _state->get_path().node_name(), false, true);
}

}  // namespace dsa