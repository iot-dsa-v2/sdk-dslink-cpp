#include "dsa_common.h"

#include "sys_root.h"

namespace dsa {
SysRoot::SysRoot(LinkStrandRef &&strand)
  : NodeModel(std::move(strand)) {

}
}
