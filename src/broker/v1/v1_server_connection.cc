#include "dsa_common.h"

#include "v1_server_connection.h"
#include "v1_session_manager.h"

namespace dsa {

V1ServerConnection::V1ServerConnection(shared_ptr_<V1SessionManager>& manager)
    : _manager(manager) {}
void V1ServerConnection::post_in_strand(std::function<void()>&& callback) {
  _manager->_strand_state_manager->post(std::move(callback));
}
V1ServerConnection::~V1ServerConnection() = default;

void V1ServerConnection::destroy_impl() { _manager.reset(); }

}  // namespace dsa