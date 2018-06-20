#ifndef DSA_SDK_NETWORK_SERVER_H
#define DSA_SDK_NETWORK_SERVER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "editable_strand.h"

#include "core/shared_strand_ref.h"
#include "util/enable_shared.h"

namespace dsa {
class Connection;

class Server : public SharedStrandPtr<Server> {
 protected:
  // for Session/Requester/Responder

  // for ServerConnection
  SharedLinkStrandRef _shared_strand;
  string_ _dsid_prefix;

  //  void on_session_connected(const ref_ <Session> session);

 public:
  explicit Server(WrapperStrand &config);
  virtual ~Server() = default;

  void post_in_strand(std::function<void()> &&callback,
                      bool already_locked) override {
    return _shared_strand->post(std::move(callback));
  }

  const string_ &get_dsid_prefix() const { return _dsid_prefix; }

  virtual void start() = 0;

  virtual string_ type() = 0;
};

typedef shared_ptr_<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H
