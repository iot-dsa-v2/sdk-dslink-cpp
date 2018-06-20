#include "dsa_common.h"

#include "v1_server_connection.h"
#include "v1_session_manager.h"

namespace dsa {

V1ServerConnection::V1ServerConnection(const SharedLinkStrandRef strand)
    : _strand(strand) {}
void V1ServerConnection::post_in_strand(std::function<void()>&& callback,
                                        bool) {
  _strand->post(std::move(callback));
}
V1ServerConnection::~V1ServerConnection() = default;

void V1ServerConnection::destroy_impl() {}

}  // namespace dsa
