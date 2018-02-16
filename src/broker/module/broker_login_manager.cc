#include "dsa_common.h"

#include <dslink.h>
#include "broker_login_manager.h"

namespace dsa {

void BrokerLoginManager::check_login(const string_ &username,
                                     const string_ &password,
                                     ClientInfo::GetClientCallback &&callback) {

}
void BrokerLoginManager::get_user(const string_ &username,
                                  ClientInfo::GetClientCallback &&callback) {
  // TODO check username and set dsid for client info accordingly
  callback(ClientInfo(), false);
}
}
