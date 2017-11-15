#include "dsa_common.h"

#include "config.h"

#include "module/logger.h"
#include "module/security_manager.h"
#include "responder/node_state_manager.h"
#include "session_manager.h"

namespace dsa {
LinkConfig::LinkConfig(boost::asio::io_service::strand* strand,
                       std::unique_ptr<ECDH>&& ecdh)
    : LinkStrand(strand, ecdh.get()),
      _ecdh(std::move(ecdh)){

      };
LinkConfig::~LinkConfig() = default;

void LinkConfig::set_security_manager(ref_<SecurityManager> p) {
  __security_manager = p.get();
  _security_manager = std::move(p);
};
void LinkConfig::set_stream_acceptor(ref_<OutgoingStreamAcceptor> p) {
  __stream_acceptor = p.get();
  _stream_acceptor = std::move(p);
};
void LinkConfig::set_session_manager(ref_<SessionManager> p) {
  __session_manager = p.get();
  _session_manager = std::move(p);
};

void LinkConfig::set_logger(std::unique_ptr<Logger> p) {
  __logger = p.get();
  _logger = std::move(p);
};

void LinkConfig::set_responder_model(ModelRef&& root_model,
                                     size_t timer_interval) {
  set_stream_acceptor(make_ref_<NodeStateManager>(
      *this, std::move(root_model), timer_interval));
}

}  // namespace dsa
