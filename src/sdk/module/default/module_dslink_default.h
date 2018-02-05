
#ifndef DSA_SDK_MODULE_DSLINK_DEFAULT_H
#define DSA_SDK_MODULE_DSLINK_DEFAULT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../module.h"

namespace dsa {
class ModuleDslinkDefault : public Module {

 protected:
  ref_<Storage> create_storage(App& app, ref_<LinkStrand> strand) override;
  shared_ptr_<Logger> create_logger(App& app, ref_<LinkStrand> strand) override;
  ref_<ClientManager> create_client_manager(App& app, ref_<LinkStrand> strand) override;
  ref_<Authorizer> create_authorizer(App& app, ref_<LinkStrand> strand) override;
};

}

#endif //DSA_SDK_MODULE_DSLINK_DEFAULT_H
