#include "dsa_common.h"

#include "config.h"

namespace dsa {
LinkConfig::LinkConfig(Strand* strand, ECDH* ecdh)
    : LinkStrand(strand, ecdh){

      };
}  // namespace dsa
