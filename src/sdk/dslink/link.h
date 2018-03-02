#ifndef DSA_DSLINK_LINK_H
#define DSA_DSLINK_LINK_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/client.h"
#include "core/editable_strand.h"
#include "core/session.h"
#include "list_merger.h"
#include "module/default/simple_storage.h"
#include "module/module.h"
#include "network/ws/ws_connection.h"
#include "subscribe_merger.h"

namespace dsa {
class App;
class TcpServer;
class LinkRoot;

class DsLinkRequester : public WrapperStrand {
 public:
  virtual ref_<IncomingSubscribeCache> subscribe(
      const string_ &path, IncomingSubscribeCache::Callback &&callback,
      const SubscribeOptions &options = SubscribeOptions::default_options) = 0;

  virtual ref_<IncomingListCache> list(
      const string_ &path, IncomingListCache::Callback &&callback) = 0;

  virtual ref_<IncomingListCache> list_raw(
      const string_ &path, IncomingListCache::Callback &&callback) = 0;

  virtual ref_<IncomingInvokeStream> invoke(
      IncomingInvokeStreamCallback &&callback,
      ref_<const InvokeRequestMessage> &&message) = 0;

  virtual ref_<IncomingSetStream> set(
      IncomingSetStreamCallback &&callback,
      ref_<const SetRequestMessage> &&message) = 0;
};

class DsLink final : public DsLinkRequester {
  friend class SubscribeMerger;
  friend class ListMerger;

 public:
  typedef std::function<void(IncomingListCache &,
                             const std::vector<std::string> &)>
      ListCallback;
  typedef std::function<void(const shared_ptr_<Connection> &,
                             ref_<DsLinkRequester>)>
      LinkOnConnectCallback;

 public:
  DsLink(int argc, const char *argv[], const string_ &link_name,
         const string_ &version, const shared_ptr_<App> &app = nullptr,
         ref_<Module> &&default_module = nullptr,
         bool use_standard_node_structure = true);
  ~DsLink() final;
  App &get_app();
  string_ get_close_token();

 protected:
  void destroy_impl() final;

 private:
  bool own_app;
  shared_ptr_<App> _app;
  shared_ptr_<TcpServer> _tcp_server;
  ref_<Client> _client;
  ref_<LinkRoot> _root;

  uint8_t log_level_from_settings;
  ref_<Module> modules;
  string_ close_token;

  bool _running = false;
  bool _connected = false;
  std::unique_ptr<SimpleSafeStorageBucket> config_bucket;

  // initialization
  void parse_thread(size_t thread);
  void parse_url(const string_ &url);
  void parse_name(const string_ &name);
  void parse_server_port(uint16_t port);

  void init_module(ref_<Module> &&default_module, const string_ &module_path,
                   bool use_standard_node_structure);

 public:
  // init raw responder root node, the dslink won't have the default standard
  // node structure
  void init_responder_raw(ref_<NodeModelBase> &&root_node);
  // init the responder's main node;
  void init_responder(ref_<NodeModelBase> &&main_node = nullptr);
  template <class NodeClass>
  void init_responder() {
    init_responder(make_ref_<NodeClass>(strand));
  }

  ref_<NodeModelBase> add_to_main_node(const string_ &name,
                                       ref_<NodeModelBase> &&node);
  void remove_from_main_node(const string_ &name);

  ref_<NodeModel> add_to_pub(const string_ &path, ref_<NodeModel> &&node);

  // the on_connect callback will always be called from main strand
  void run(DsLink::LinkOnConnectCallback &&on_connect = nullptr,
           uint8_t callback_type = 1 /*Client::FIRST_CONNECTION*/);
  // if app is wanted to be used later,
  // only connect should be called and
  // run should  be called manually again and
  void connect(DsLink::LinkOnConnectCallback &&on_connect = nullptr,
               uint8_t callback_type = 1 /*Client::FIRST_CONNECTION*/);

  SimpleSafeStorageBucket &get_config_bucket() { return *config_bucket; };
  // requester functions
 private:
  std::unordered_map<std::string, ref_<SubscribeMerger>> _subscribe_mergers;
  std::unordered_map<std::string, ref_<ListMerger>> _list_mergers;

 private:
  ref_<IncomingSubscribeCache> subscribe(
      const string_ &path, IncomingSubscribeCache::Callback &&callback,
      const SubscribeOptions &options =
          SubscribeOptions::default_options) final;

  ref_<IncomingListCache> list(const string_ &path,
                               IncomingListCache::Callback &&callback) final;

  ref_<IncomingListCache> list_raw(
      const string_ &path, IncomingListCache::Callback &&callback) final;

  ref_<IncomingInvokeStream> invoke(
      IncomingInvokeStreamCallback &&callback,
      ref_<const InvokeRequestMessage> &&message) final;

  ref_<IncomingSetStream> set(IncomingSetStreamCallback &&callback,
                              ref_<const SetRequestMessage> &&message) final;
};
}  // namespace dsa

#endif  // DSA_DSLINK_LINK_H
