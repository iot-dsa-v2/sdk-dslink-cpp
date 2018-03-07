#include "dsa_common.h"

#include "broker_client_manager.h"
#include "broker_client_nodes.h"
#include "module/storage.h"
#include "module/stream_acceptor.h"

namespace dsa {

BrokerClientsRoot::BrokerClientsRoot(LinkStrandRef&& strand,
                                     ref_<BrokerClientManager>&& manager)
    : NodeModel(std::move(strand)),
      _manager(std::move(manager)),
      _storage(_strand->storage().get_bucket("Clients")){};

void BrokerClientsRoot::initialize() {
  _storage->read_all([ this, keepref = get_ref() ](
                         const string_& key, std::vector<uint8_t> data,
                         BucketReadStatus read_status) mutable {
    if (PathData::invalid_name(key)) {
      // TODO check dsid
      return;
    }
    Var map =
        Var::from_json(reinterpret_cast<const char*>(data.data()), data.size());

    // TODO, remove this dispatch, as well as the above mutable, just a work
    // around before we have strand bucket
    if (map.is_map()) {
      _strand->dispatch(
          [ this, keepref = std::move(keepref), key, map = std::move(map) ]() {
            // add a child dslink node
            auto child = make_ref_<BrokerClientNode>(
                _strand->get_ref(),
                _strand->stream_acceptor().get_profile("Broker/Client", true));

            child->load(map.get_map());

            add_list_child(key, std::move(child));
          });
    }
  },
                     [manager = _manager]() { manager->rebuild_path2id(); });
}

BrokerClientNode::BrokerClientNode(LinkStrandRef&& strand,
                                   ref_<NodeModel>&& profile)
    : NodeModel(std::move(strand), std::move(profile)){};

void BrokerClientNode::save_extra(VarMap& map) const {
  // TODO, change these to writable children value nodes
  map["?group"] = _client_info.group;
  map["?default-token"] = _client_info.default_token;
  map["?path"] = _client_info.responder_path;
  map["?max-session"] = static_cast<int64_t>(_client_info.max_session);
}
void BrokerClientNode::load_extra(VarMap& map) {
  // TODO, change these to writable children value nodes
  _client_info.group = map["?group"].to_string();
  _client_info.default_token = map["?default-token"].to_string();
  _client_info.responder_path = map["?path"].to_string();
  _client_info.max_session = static_cast<size_t>(map["?max-session"].get_int());
}

}  // namespace dsa
