#include "dsa_common.h"

#include "simple_login_manager.h"

#include "../../util/client_info.h"
#include "../../web_server/login_manager.h"

namespace dsa {

void SimpleLoginManager::check_login(const string_ &username,
                                     const string_ &password,
                                     ClientInfo::GetClientCallback &&callback) {
}

void SimpleLoginManager::get_user(const string_ &username,
                                  ClientInfo::GetClientCallback &&callback) {
  // TODO check username and set dsid for client info accordingly
  callback(ClientInfo(), false);
}
}