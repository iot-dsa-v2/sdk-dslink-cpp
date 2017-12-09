#include "dsa_common.h"

#include "pub_root.h"

namespace dsa {
BrokerPubRoot::BrokerPubRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}
