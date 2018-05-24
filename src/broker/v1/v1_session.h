#ifndef DSA_SDK_V1_SESSION_H
#define DSA_SDK_V1_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/session.h"

namespace dsa {
class V1Session : public BaseSession {
  LinkStrandRef _strand;

  string_ _salt;

 public:
  const string_ &current_salt() const { return _salt; };

  V1Session(const LinkStrandRef &strand);
  ~V1Session();

 public:
  void write_stream(ref_<MessageStream> &&stream) override;
  // this stream must be handled first
  void write_critical_stream(ref_<MessageStream> &&stream) override;

  bool destroy_resp_stream(int32_t rid) override;
  bool destroy_req_stream(int32_t rid) override;

  // requester
  ref_<IncomingSubscribeStream> subscribe(
      const string_ &path, IncomingSubscribeStreamCallback &&callback,
      const SubscribeOptions &options =
          SubscribeOptions::default_options) override;

  ref_<IncomingListStream> list(
      const string_ &path, IncomingListStreamCallback &&callback,
      const ListOptions &options = ListOptions::default_options) override;

  ref_<IncomingInvokeStream> invoke(
      IncomingInvokeStreamCallback &&callback,
      ref_<const InvokeRequestMessage> &&message) override;

  ref_<IncomingSetStream> set(IncomingSetStreamCallback &&callback,
                              ref_<const SetRequestMessage> &&message) override;

  bool is_connected() const override;
  string_ map_pub_path(const string_ &path) override;
};
}  // namespace dsa

#endif  // DSA_SDK_V1_SESSION_H
