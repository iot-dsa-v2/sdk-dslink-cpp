#include "dsa_common.h"

#include "pub_root.h"

namespace dsa {
LinkPubRoot::LinkPubRoot(LinkStrandRef &&strand)
    : NodeModel(std::move(strand)) {}
LinkPubRoot::~LinkPubRoot() = default;
}
