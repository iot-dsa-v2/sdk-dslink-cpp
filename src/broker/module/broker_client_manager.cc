#include "dsa_common.h"

#include "broker_client_manager.h"

#include "module/stream_acceptor.h"
namespace dsa {

void BrokerLinksRoot::initialize() {
  auto storage = _strand->storage().get_bucket("Users");
  storage->read_all([ this, keepref = get_ref() ](
                        const string_& key, std::vector<uint8_t> data,
                        BucketReadStatus read_status) {
                        Path path(key);

                      },
                    []() {

                    });
}
void BrokerLinksRoot::on_load_child(const string_& name, VarMap& map) {
  auto user_node = make_ref_<BrokerLinkNode>(
      _strand->get_ref(),
      _strand->stream_acceptor().get_profile("Broker/Link", true));
  add_list_child(name, std::move(user_node));
}

void BrokerClientManager::create_node(NodeModel& module_node,
                                      BrokerPubRoot& pub_root) {
  // TODO register action for pub root
}

void BrokerClientManager::get_client(const string_& dsid,
                                     const string_& auth_token,
                                     ClientInfo::GetClientCallback&& callback) {
  _strand->post([ dsid, auth_token, callback = std::move(callback) ]() {
    ClientInfo rslt(dsid, auth_token);
    rslt.responder_path =
        string_("downstream/") + dsid.substr(0, dsid.length() - 43);
    callback(std::move(rslt), false);
  });
}

void BrokerClientManager::destroy_impl() override {
  _links_node.reset();
  ClientManager::destroy_impl();
}
}
