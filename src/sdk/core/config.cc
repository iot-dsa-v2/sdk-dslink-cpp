#include "dsa_common.h"

#include "config.h"

namespace dsa {
LinkConfig::LinkConfig(boost::asio::io_service::strand* strand, ECDH* ecdh)
    : LinkStrand(strand, ecdh){

      };
}  // namespace dsa
