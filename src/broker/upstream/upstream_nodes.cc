#include "dsa_common.h"

#include "upstream_nodes.h"

#include "core/client.h"
#include "module/stream_acceptor.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"

namespace dsa {

UpstreamRootNode::UpstreamRootNode(const LinkStrandRef &strand)
    : NodeModel(strand),
      _storage(_strand->storage().get_strand_bucket("Upstreams", _strand)) {}

void UpstreamRootNode::destroy_impl() {
  _storage.reset();
  NodeModel::destroy_impl();
}

void UpstreamRootNode::initialize() {
  NodeModel::initialize();

  _storage->read_all(  //
      [ this, keepref = get_ref() ](const string_ &key,
                                    std::vector<uint8_t> data,
                                    BucketReadStatus read_status) mutable {
        if (PathData::invalid_name(key)) {
          return;
        }
        Var map = Var::from_json(reinterpret_cast<const char *>(data.data()),
                                 data.size());

        if (map.is_map()) {
          // add a upstream connection

          ref_<UpstreamConnectionNode> child =
              make_ref_<UpstreamConnectionNode>(
                  _strand, get_ref(),
                  _strand->stream_acceptor().get_profile(
                      "Broker/Upstream_Connection", true));

          child->load(map.get_map());
          add_list_child(key, child->get_ref());
        }
      },
      []() {});
}

UpstreamConnectionNode::UpstreamConnectionNode(const LinkStrandRef &strand,
                                               ref_<UpstreamRootNode> &&parent,
                                               ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)), _parent(std::move(parent)) {}
UpstreamConnectionNode::~UpstreamConnectionNode() = default;

void UpstreamConnectionNode::destroy_impl() {
  if (_client != nullptr) {
    _client->destroy();
    _client.reset();
  }
  _parent.reset();

  _enabled_node.reset();
  _name_on_local_node.reset();
  _url_node.reset();
  _name_on_remote_node.reset();
  _token_node.reset();
  _role_node.reset();

  NodeModel::destroy_impl();
}

void UpstreamConnectionNode::save_upstream() const {
  save(*_parent->_storage, _state->get_path().node_name(), false, true);
}
void UpstreamConnectionNode::save_extra(VarMap &map) const {}
void UpstreamConnectionNode::load_extra(VarMap &map) {}

}  // namespace dsa
