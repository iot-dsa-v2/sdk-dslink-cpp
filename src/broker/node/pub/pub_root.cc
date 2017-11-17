#include "dsa_common.h"

#include "pub_root.h"

namespace dsa {
PubRoot::PubRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}
