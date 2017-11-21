#include "dsa_common.h"

#include "downstream_root.h"

namespace dsa {
DownstreamRoot::DownstreamRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}