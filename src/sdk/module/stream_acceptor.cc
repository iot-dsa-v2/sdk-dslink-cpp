#include "dsa_common.h"

#include "stream_acceptor.h"

#include "responder/node_model.h"

namespace dsa {
ref_<NodeModel> OutgoingStreamAcceptor::get_profile(const string_ &path,
                                                            bool dsa_standard) {
  return ref_<NodeModel>();
}

}
