#include "dsa_common.h"

#include "config.h"

namespace dsa {
LinkConfig::LinkConfig(AsioStrand* strand, ECDH* ecdh)
    : LinkStrand(strand, ecdh){

      };
}  // namespace dsa
