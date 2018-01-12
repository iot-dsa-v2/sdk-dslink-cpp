#ifndef DSA_SDK_LINK_STRAND_H
#define DSA_SDK_LINK_STRAND_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"

namespace boost {
namespace asio {
class io_context;
}
}

namespace dsa {

class SecurityManager;
class SessionManager;
class OutgoingStreamAcceptor;
class ECDH;
class Logger;
class StrandTimer;
typedef ref_<StrandTimer> TimerRef;

class LinkStrand : public DestroyableRef<LinkStrand> {
 public:
  // input : canceled
  // output : need repeat
  typedef std::function<bool(bool)> TimerCallback;

 protected:
  // managed pointer by LinkStrand
  // void pointer because there is no way to forward declare the strand type
  void *__strand = nullptr;

  // following pointers are managed by EditableStrand

  ECDH *__ecdh = nullptr;

  SecurityManager *__security_manager = nullptr;
  OutgoingStreamAcceptor *__stream_acceptor = nullptr;
  SessionManager *__session_manager = nullptr;
  Logger *__logger = nullptr;

  void destroy_impl() override;

 public:
  explicit LinkStrand(void *strand, ECDH *ecdh);

  virtual ~LinkStrand();

  void *asio_strand() { return __strand; }
  boost::asio::io_context &get_io_context();
  void post(std::function<void()> &&);
  void dispatch(std::function<void()> &&);

  // inject a function and run it as soon as possible
  virtual void inject(std::function<void()> &&) = 0;
  // run all the injected callbacks
  virtual void check_injected() = 0;

  ref_<StrandTimer> add_timer(int32_t interval_ms, TimerCallback &&callback);

  SecurityManager &security_manager() { return *__security_manager; };

  OutgoingStreamAcceptor &stream_acceptor() { return *__stream_acceptor; };

  SessionManager &session_manager() { return *__session_manager; };

  Logger &logger() { return *__logger; };

  ECDH &ecdh() { return *__ecdh; };
};
typedef ref_<LinkStrand> LinkStrandRef;
}  // namespace dsa
#endif  // DSA_SDK_LINK_STRAND_H
