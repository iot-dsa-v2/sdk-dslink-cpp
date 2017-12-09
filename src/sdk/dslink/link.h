#ifndef DSA_DSLINK_LINK_H
#define DSA_DSLINK_LINK_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/client.h"
#include "core/editable_strand.h"
#include "core/session.h"
#include "list_merger.h"
#include "subscribe_merger.h"

namespace dsa {
class App;
class TcpServer;
class LinkRoot;

class DsLink final : public WrapperStrand {
  friend class SubscribeMerger;
  friend class ListMerger;

 public:
  typedef std::function<void(IncomingListCache &,
                             const std::vector<std::string> &)>
      ListCallback;

 public:
  DsLink(int argc, const char *argv[], const string_ &link_name,
         const string_ &version, const shared_ptr_<App> &app = nullptr);
  ~DsLink() final;
  App &get_app();

 protected:
  void destroy_impl() final;

 private:
  shared_ptr_<App> _app;
  shared_ptr_<TcpServer> _tcp_server;
  ref_<Client> _client;
  ref_<LinkRoot> _root;

  bool _running = false;
  bool _connected = false;

  // initialization
  void parse_thread(size_t thread);
  void parse_url(const string_ &url);
  void parse_log(const string_ &log, EditableStrand &config);
  void parse_name(const string_ &name);
  void parse_server_port(uint16_t port);

 public:
  void init_responder_main(ref_<NodeModelBase> &&main_node = nullptr);
  template <class NodeClass>
  void init_responder_main() {
    init_responder_main(make_ref_<NodeClass>(strand));
  }

  // the on_connect callback will always be called from main strand
  void run(Client::OnConnectCallback &&on_connect = nullptr,
           uint8_t callback_type = 1 /*Client::FIRST_CONNECTION*/);
  // if app is wanted to be used later,
  // only connect should be called and
  // run should  be called manually again and
  void connect(Client::OnConnectCallback &&on_connect = nullptr,
               uint8_t callback_type = 1 /*Client::FIRST_CONNECTION*/);

  // requester functions
 private:
  std::unordered_map<std::string, ref_<SubscribeMerger>> _subscribe_mergers;
  std::unordered_map<std::string, ref_<ListMerger>> _list_mergers;

 public:
  ref_<IncomingSubscribeCache> subscribe(
      const string_ &path, IncomingSubscribeCache::Callback &&callback,
      const SubscribeOptions &options = SubscribeOptions::default_options);

  ref_<IncomingListCache> list(const string_ &path,
                               IncomingListCache::Callback &&callback);

  ref_<IncomingInvokeStream> invoke(IncomingInvokeStreamCallback &&callback,
                                    ref_<const InvokeRequestMessage> &&message);

  ref_<IncomingSetStream> set(IncomingSetStreamCallback &&callback,
                              ref_<const SetRequestMessage> &&message);
};
}

#endif  // DSA_DSLINK_LINK_H
