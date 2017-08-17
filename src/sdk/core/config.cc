#include "dsa_common.h"

#include "config.h"

namespace dsa {
Config::Config(intrusive_ptr_<ECDH> ecdh, boost::asio::io_service::strand &strand) 
	: ecdh(ecdh), strand(strand) {}
}  // namespace dsa
