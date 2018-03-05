#include "dsa_common.h"

#include "broker_client_manager.h"

#include "module/stream_acceptor.h"
#include "responder/value_node_model.h"

namespace dsa {

void BrokerKnownLinksRoot::initialize() {
  auto storage = _strand->storage().get_bucket("Known_Links");
  storage->read_all([ this, keepref = get_ref() ](
                        const string_& key, std::vector<uint8_t> data,
                        BucketReadStatus read_status) {
    if (PathData::invalid_name(key)) {
      // TODO check dsid
      return;
    }
    Var map =
        Var::from_json(reinterpret_cast<const char*>(data.data()), data.size());
    if (map.is_map()) {
      // add a child dslink node
      auto child = make_ref_<BrokerKnownLinkNode>(
          _strand->get_ref(),
          _strand->stream_acceptor().get_profile("Broker/Known_Link", true));

      child->load(map.get_map());
      add_list_child(key, std::move(child));
    }
  },
                    nullptr);
}

void BrokerClientManager::create_nodes(NodeModel& module_node,
                                       BrokerPubRoot& pub_root) {
  // TODO register action for pub root

  _known_links.reset(new BrokerKnownLinksRoot(_strand->get_ref()));
  _quarantine.reset(new NodeModel(_strand->get_ref()));
  _quarantine->add_list_child(
      "Enabled", make_ref_<ValueNodeModel>(_strand->get_ref(),
                                           [](const Var&) { return true; },
                                           PermissionLevel::CONFIG));
}

void BrokerClientManager::get_client(const string_& dsid,
                                     const string_& auth_token,
                                     ClientInfo::GetClientCallback&& callback) {
  _strand->post([ dsid, auth_token, callback = std::move(callback) ]() {
    ClientInfo rslt(dsid, auth_token);
    rslt.responder_path =
        string_("Downstream/") + dsid.substr(0, dsid.length() - 43);
    callback(std::move(rslt), false);
  });
}

void BrokerClientManager::destroy_impl() {
  _known_links.reset();
  _quarantine.reset();
  ClientManager::destroy_impl();
}
}  // namespace dsa
