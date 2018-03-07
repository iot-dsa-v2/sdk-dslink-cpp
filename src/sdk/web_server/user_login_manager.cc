#include "user_login_manager.h"

namespace dsa {

UserLoginManager::UserLoginManager(WebServer &web_server)
    : _web_server(web_server) {}

void UserLoginManager::check_login(const string_ &username,
                                   const string_ &password,
                                   ClientInfo::GetClientCallback &&callback) {
  _web_server.io_service().post([ username, callback = std::move(callback) ]() {
    callback(ClientInfo(username), false);
  });
}

void UserLoginManager::get_user(const string_ &username,
                                ClientInfo::GetClientCallback &&callback) {
  _web_server.io_service().post([ username, callback = std::move(callback) ]() {
    callback(ClientInfo(username), false);
  });
}
}