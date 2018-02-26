#include "dsa_common.h"

#include "module_dslink_default.h"

#include "../storage.h"
#include "console_logger.h"
#include "simple_login_manager.h"
#include "simple_security.h"
#include "simple_session_manager.h"

#include "../../web_server/login_manager.h"
#include "module/authorizer.h"
#include "module/client_manager.h"
#include "module/logger.h"
#include "module/storage.h"

namespace dsa {

ref_<Storage> ModuleDslinkDefault::create_storage(App& app,
                                                  ref_<LinkStrand>& strand) {
  // return make_ref_<SimpleStorage>(&app.io_service());
  return nullptr;
}

shared_ptr_<Logger> ModuleDslinkDefault::create_logger(
    App& app, ref_<LinkStrand>& strand) {
  return make_shared_<ConsoleLogger>();
}

ref_<ClientManager> ModuleDslinkDefault::create_client_manager(
    App& app, ref_<LinkStrand>& strand) {
  return make_ref_<SimpleClientManager>();
}

ref_<Authorizer> ModuleDslinkDefault::create_authorizer(
    App& app, ref_<LinkStrand>& strand) {
  return make_ref_<SimpleAuthorizer>(strand);
}

shared_ptr_<LoginManager> ModuleDslinkDefault::create_login_manager(
    App& app, ref_<LinkStrand>& strand) {
  return make_shared_<SimpleLoginManager>(strand);
}
}
