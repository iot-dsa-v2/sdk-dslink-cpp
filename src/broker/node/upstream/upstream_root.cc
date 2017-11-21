#include "dsa_common.h"

#include "upstream_root.h"

namespace dsa {
UpstreamRoot::UpstreamRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}
