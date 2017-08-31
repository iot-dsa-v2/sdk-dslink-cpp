#include "dsa_common.h"

#include "link_strand.h"



#include "crypto/ecdh.h"

namespace dsa {
LinkStrand::LinkStrand(AsioStrand* strand, ECDH* ecdh)
  : __strand(strand), __ecdh(ecdh) {}
LinkStrand::~LinkStrand() {
  delete __strand;
  delete __ecdh;
}

}
