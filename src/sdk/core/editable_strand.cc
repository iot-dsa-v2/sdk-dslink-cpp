#include "dsa_common.h"

#include "editable_strand.h"

#include <regex>
#include "module/authorizer.h"
#include "module/client_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "module/logger.h"
#include "module/session_manager.h"
#include "network/tcp/stcp_client_connection.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/ws/ws_client_connection.h"
#include "responder/node_state_manager.h"
#include "util/app.h"
#include "util/certificate.h"

namespace dsa {

ref_<EditableStrand> EditableStrand::make_default(shared_ptr_<App> app) {
  auto strand =
      make_ref_<EditableStrand>(app->new_strand(), make_unique_<ECDH>());

  strand->set_session_manager(make_ref_<SimpleSessionManager>(strand));

  strand->set_client_manager(make_ref_<SimpleClientManager>());
  strand->set_authorizer(make_ref_<SimpleAuthorizer>(strand));

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
  INJECT_QUEUE_TYPE temp;
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

void WrapperStrand::set_client_connection_maker() {
  static boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
  boost::system::error_code error_code;

  if (tcp_port > 0 && secure) {
    load_root_certificate(context, error_code);

    client_connection_maker =
        [ dsid_prefix = dsid_prefix, tcp_host = tcp_host,
          tcp_port = tcp_port ](const SharedLinkStrandRef& strand)
            ->shared_ptr_<Connection> {
      return make_shared_<StcpClientConnection>(strand, context, dsid_prefix,
                                                tcp_host, tcp_port);
    };
    return;
  }
  if (ws_port > 0) {
    client_connection_maker =
        [
          dsid_prefix = dsid_prefix, ws_host = ws_host, ws_port = ws_port,
          secure = secure
        ](const SharedLinkStrandRef& strand)
            ->shared_ptr_<Connection> {
      return make_shared_<WsClientConnection>(secure, strand, dsid_prefix,
                                              ws_host, ws_port);
    };
    return;
  }
  client_connection_maker =
      [ dsid_prefix = dsid_prefix, tcp_host = tcp_host,
        tcp_port = tcp_port ](const SharedLinkStrandRef& strand)
          ->shared_ptr_<Connection> {
    return make_shared_<TcpClientConnection>(strand, dsid_prefix, tcp_host,
                                             tcp_port);
  };
}

bool WrapperStrand::parse_url(const string_& url) {
  static std::regex url_regex(
      R"(^(ds://|dss://|ws://|wss://)?([^/:\[]+|\[[0-9A-Fa-f:]+\])(:\d+)?(/.*)?$)");

  auto match = std::sregex_iterator(url.begin(), url.end(), url_regex);
  if (match == std::sregex_iterator()) {  // match is a empty iterator
    return false;
  }
  string_ protocol = (*match)[1].str();

  if (protocol == "ws://" || protocol == "wss://") {
    if (protocol == "wss://") {
      secure = true;
      ws_port = 443;  // default wss port
    } else {
      ws_port = 80;  // default ws port
    }
    ws_host = (*match)[2].str();
    if ((*match)[3].length() > 1) {
      ws_port = static_cast<uint16_t>(
          std::stoi(string_((*match)[3].first + 1, (*match)[3].second)));
    }
    ws_path = (*match)[4].str();
  } else {
    if (protocol == "dss://") {
      secure = true;
      tcp_port = 4128;  // default dss port
    } else {            // "ds://" or blank
      tcp_port = 4120;  // default ds port
    }
    tcp_host = (*match)[2].str();
    if ((*match)[3].length() > 1) {
      tcp_port = static_cast<uint16_t>(
          std::stoi(string_((*match)[3].first + 1, (*match)[3].second)));
    }
  }
  return true;
}
}  // namespace dsa
