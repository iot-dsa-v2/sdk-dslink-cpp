#include "dsa_common.h"

#include "broker_client_manager.h"

namespace dsa {

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
}
