#include "dsa_common.h"

#include "pub_root.h"

namespace dsa {
BrokerPubRoot::BrokerPubRoot(LinkStrandRef &&strand, const string_ &profile)
    : PubRoot(std::move(strand), profile) {}
}
