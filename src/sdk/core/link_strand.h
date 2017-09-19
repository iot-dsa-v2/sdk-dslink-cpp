#ifndef DSA_SDK_LINK_STRAND_H
#define DSA_SDK_LINK_STRAND_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_intrusive.h"

namespace dsa {

class SecurityManager;
class SessionManager;
class OutgoingStreamAcceptor;
class ECDH;
class Logger;

class LinkStrand : public EnableRef<LinkStrand> {
 protected:
  // managed pointer by LinkStrand
  void *__strand = nullptr;
  ECDH *__ecdh = nullptr;

  SecurityManager *__security_manager = nullptr;
  OutgoingStreamAcceptor *__stream_acceptor = nullptr;
  SessionManager *__session_manager = nullptr;
  Logger *__logger = nullptr;

 public:
  explicit LinkStrand(void *strand, ECDH *ecdh);

  virtual ~LinkStrand();

  void *asio_strand() { return __strand; }
  void post(std::function<void()> &&);
  void dispatch(std::function<void()> &&);

  SecurityManager &security_manager() { return *__security_manager; };

  OutgoingStreamAcceptor &stream_acceptor() { return *__stream_acceptor; };

  SessionManager &session_manager() { return *__session_manager; };

  Logger &logger() { return *__logger; };

  ECDH &ecdh() { return *__ecdh; };
};
typedef ref_<LinkStrand> LinkStrandRef;
}  // namespace dsa
#endif  // DSA_SDK_LINK_STRAND_H
