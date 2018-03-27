#include "dsa_common.h"

#include "editable_strand.h"

#include "module/authorizer.h"
#include "module/client_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "module/logger.h"
#include "module/session_manager.h"
#include "responder/node_state_manager.h"
#include "util/app.h"

namespace dsa {

ref_<EditableStrand> EditableStrand::make_default(shared_ptr_<App> app) {
  auto strand =
      make_ref_<EditableStrand>(app->new_strand(), make_unique_<ECDH>());

  strand->set_session_manager(make_ref_<SimpleSessionManager>(strand));

  strand->set_client_manager(make_ref_<SimpleClientManager>());
  strand->set_authorizer(make_ref_<SimpleAuthorizer>(strand));

  static_cast<ConsoleLogger&>(Logger::_()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;
  Logger::_().level = Logger::WARN__;

  return strand;
}

EditableStrand::EditableStrand(boost::asio::io_context::strand* strand,
                               std::unique_ptr<ECDH>&& ecdh)
    : LinkStrand(strand, ecdh.get()), _ecdh(std::move(ecdh)) {
  _prepare_inject_callback();
};
EditableStrand::~EditableStrand() = default;

void EditableStrand::set_client_manager(ref_<ClientManager> p) {
  __client_manager = p.get();
  _client_manager = std::move(p);
};
void EditableStrand::set_authorizer(ref_<Authorizer> p) {
  __authorizer = p.get();
  _authorizer = std::move(p);
}
void EditableStrand::set_stream_acceptor(ref_<OutgoingStreamAcceptor> p) {
  __stream_acceptor = p.get();
  _stream_acceptor = std::move(p);
};
void EditableStrand::set_session_manager(ref_<SessionManager> p) {
  __session_manager = p.get();
  _session_manager = std::move(p);
};

void EditableStrand::set_storage(ref_<Storage> p) {
  __storage = p.get();
  _storage = std::move(p);
};

void EditableStrand::set_responder_model(ModelRef&& root_model,
                                         size_t timer_interval) {
  set_stream_acceptor(make_ref_<NodeStateManager>(*this, std::move(root_model),
                                                  timer_interval));
}

void EditableStrand::destroy_impl() {
  LinkStrand::destroy_impl();
  // destroy of following objects is handled in LinkStrand::destroy_impl()
  // but allow logger and storage continue to work without the strand
  _session_manager.reset();
  _stream_acceptor.reset();
  _client_manager.reset();
  _authorizer.reset();

  {
    std::lock_guard<std::mutex> lock(_inject_mutex);
    _inject_callback = nullptr;
    _inject_queue.clear();
  }
}

void EditableStrand::check_injected() {
  if (_inject_queue.empty()) return;
  std::vector<std::function<void()>> temp;
  {
    std::lock_guard<std::mutex> lock(_inject_mutex);
    std::swap(_inject_queue, temp);
  }
  for (auto& callback_it : temp) {
    callback_it();
  }
}

void EditableStrand::_prepare_inject_callback() {
  std::lock_guard<std::mutex> lock(_inject_mutex);
  _inject_callback = [ this, keep_ref = get_ref() ]() {
    if (is_destroyed()) return;
    _prepare_inject_callback();
    check_injected();
  };
}
void EditableStrand::inject(std::function<void()>&& callback) {
  DSA_REF_GUARD;
  std::lock_guard<std::mutex> lock(_inject_mutex);

  if (_inject_callback != nullptr) {
    _inject_queue.emplace_back(std::move(callback));
    post(std::move(_inject_callback));
  } else if (is_destroyed()) {
    // no need to call it
    // just destroy the callback in strand
    post([callback = std::move(callback)](){});
  } else {
    // callback not ready, but still need to put it in queue
    _inject_queue.emplace_back(std::move(callback));
  }
}

string_ WrapperStrand::get_dsid() const {
  return strand->ecdh().get_dsid(dsid_prefix);
}
}  // namespace dsa
