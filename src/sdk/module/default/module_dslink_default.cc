#include "dsa_common.h"

#include "module_dslink_default.h"

#include "console_logger.h"
#include "simple_session_manager.h"
#include "simple_security.h"
#include "simple_storage.h"

namespace dsa {

ref_<Storage> ModuleDslinkDefault::create_storage(App& app, ref_<LinkStrand> strand){
  // TODO: I am not sure, check it
  return make_ref_<SimpleStorage>(&app.io_service());
}

ref_<Logger> ModuleDslinkDefault::create_logger(App& app, ref_<LinkStrand> strand){
  return make_ref_<ConsoleLogger>();
}

ref_<ClientManager> ModuleDslinkDefault::create_client_manager(App& app, ref_<LinkStrand> strand){
  return make_ref_<SimpleClientManager>();
}

ref_<Authorizer> ModuleDslinkDefault::create_authorizer(App& app, ref_<LinkStrand> strand){
  return make_ref_<SimpleAuthorizer>(strand);
}

}



