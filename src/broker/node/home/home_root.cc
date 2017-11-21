#include "dsa_common.h"

#include "home_root.h"

namespace dsa {
HomeRoot::HomeRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}