#include "dsa_common.h"

#include "downstream_root.h"

#include "../../remote_node/remote_root_node.h"
#include "core/session.h"
#include "util/client_info.h"
#include "util/string.h"

namespace dsa {

static const string_ DOWNSRTEAM_PATH = "Downstream/";
static const size_t DOWNSTREAM_PATH_LEN = DOWNSRTEAM_PATH.size();

DownstreamRoot::DownstreamRoot(LinkStrandRef &&strand)
    : NodeModel(std::move(strand)) {}
}