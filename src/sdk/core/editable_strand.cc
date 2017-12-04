#include "dsa_common.h"

#include "editable_strand.h"

#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "module/default/simple_session_manager.h"
#include "module/logger.h"
#include "module/security_manager.h"
#include "module/session_manager.h"
#include "responder/node_state_manager.h"
#include "util/app.h"

namespace dsa {

ref_<EditableStrand> EditableStrand::make_default(shared_ptr_<App> app) {
  auto config =
      make_ref_<EditableStrand>(app->new_strand(), make_unique_<ECDH>());

  config->set_session_manager(make_ref_<SimpleSessionManager>(config));

  config->set_security_manager(make_ref_<SimpleSecurityManager>());

  config->set_logger(make_unique_<ConsoleLogger>());
  config->logger().level = Logger::WARN__;

  return config;
}

EditableStrand::EditableStrand(boost::asio::io_service::strand* strand,
                               std::unique_ptr<ECDH>&& ecdh)
    : LinkStrand(strand, ecdh.get()),
      _ecdh(std::move(ecdh)){

      };
EditableStrand::~EditableStrand() = default;

void EditableStrand::set_security_manager(ref_<SecurityManager> p) {
  __security_manager = p.get();
  _security_manager = std::move(p);
};
void EditableStrand::set_stream_acceptor(ref_<OutgoingStreamAcceptor> p) {
  __stream_acceptor = p.get();
  _stream_acceptor = std::move(p);
};
void EditableStrand::set_session_manager(ref_<SessionManager> p) {
  __session_manager = p.get();
  _session_manager = std::move(p);
};

void EditableStrand::set_logger(std::unique_ptr<Logger> p) {
  __logger = p.get();
  _logger.swap(p);
};

void EditableStrand::set_responder_model(ModelRef&& root_model,
                                         size_t timer_interval) {
  set_stream_acceptor(make_ref_<NodeStateManager>(*this, std::move(root_model),
                                                  timer_interval));
}
void EditableStrand::destroy_impl() {
  LinkStrand::destroy_impl();
  _session_manager.reset();
  _stream_acceptor.reset();
  _security_manager.reset();
}

}  // namespace dsa
