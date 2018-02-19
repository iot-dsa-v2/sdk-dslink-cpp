#include "dsa_common.h"

#include "simple_login_manager.h"

#include "../../util/client_info.h"
#include "../../web_server/login_manager.h"

namespace dsa {

SimpleLoginManager::SimpleLoginManager(LinkStrandRef strand)
    : _strand(std::move(strand)) {}

void SimpleLoginManager::check_login(const string_ &username,
                                     const string_ &password,
                                     ClientInfo::GetClientCallback &&callback) {
  _strand->post([ username, callback = std::move(callback) ]() {
    callback(ClientInfo(username), false);
  });
}

void SimpleLoginManager::get_user(const string_ &username,
                                  ClientInfo::GetClientCallback &&callback) {
  _strand->post([ username, callback = std::move(callback) ]() {
    callback(ClientInfo(username), false);
  });
}
}