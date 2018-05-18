#include "dsa_common.h"

#include "v1_server_connection.h"

namespace dsa {

V1ServerConnection::V1ServerConnection(const SharedLinkStrandRef &strand)
    : _shared_strand(strand) {}
void V1ServerConnection::post_in_strand(std::function<void()> &&callback) {
  _shared_strand->post(std::move(callback));
}

void V1ServerConnection::destroy_impl() {
  _shared_strand.reset();
}

}  // namespace dsa