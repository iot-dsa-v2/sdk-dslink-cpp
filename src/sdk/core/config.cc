#include "dsa_common.h"

#include "config.h"

#include "module/logger.h"
#include "module/security_manager.h"
#include "responder/node_state_manager.h"
#include "session_manager.h"

namespace dsa {
LinkConfig::LinkConfig(boost::asio::io_service::strand* strand, ECDH* ecdh)
    : LinkStrand(strand, ecdh){

      };
LinkConfig::~LinkConfig() = default;

void LinkConfig::set_security_manager(std::unique_ptr<SecurityManager> p) {
  __security_manager = p.get();
  _security_manager = std::move(p);
};
void LinkConfig::set_stream_acceptor(
    std::unique_ptr<OutgoingStreamAcceptor> p) {
  __stream_acceptor = p.get();
  _stream_acceptor = std::move(p);
};
void LinkConfig::set_session_manager(std::unique_ptr<SessionManager> p) {
  __session_manager = p.get();
  _session_manager = std::move(p);
};

void LinkConfig::set_logger(std::unique_ptr<Logger> p) {
  __logger = p.get();
  _logger = std::move(p);
};

void LinkConfig::set_responder_model(ref_<NodeModelBase>&& root_model,
                                     size_t timer_interval) {
  set_stream_acceptor(make_unique_<NodeStateManager>(
      *this, std::move(root_model), timer_interval));
}

}  // namespace dsa
