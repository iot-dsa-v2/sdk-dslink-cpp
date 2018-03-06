#include "dsa_common.h"

#include "broker_client_nodes.h"
#include "module/storage.h"
#include "module/stream_acceptor.h"

namespace dsa {

void BrokerClientsRoot::initialize() {
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
      auto child = make_ref_<BrokerClientNode>(
          _strand->get_ref(),
          _strand->stream_acceptor().get_profile("Broker/Known_Link", true));

      child->load(map.get_map());
      add_list_child(key, std::move(child));
    }
  },
                    nullptr);
}

}  // namespace dsa
