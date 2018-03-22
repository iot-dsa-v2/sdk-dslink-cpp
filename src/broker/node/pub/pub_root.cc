#include "dsa_common.h"

#include "pub_root.h"

namespace dsa {
BrokerPubRoot::BrokerPubRoot(const LinkStrandRef &strand, const string_ &profile)
    : PubRoot(strand, profile) {}
}
