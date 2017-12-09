#include "dsa_common.h"

#include "home_root.h"

namespace dsa {
BrokerHomeRoot::BrokerHomeRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}