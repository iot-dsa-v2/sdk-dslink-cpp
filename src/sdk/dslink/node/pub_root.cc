#include "dsa_common.h"

#include "pub_root.h"

namespace dsa {
LinkPubRoot::LinkPubRoot(const LinkStrandRef &strand)
    : NodeModel(strand) {}
LinkPubRoot::~LinkPubRoot() = default;
}
