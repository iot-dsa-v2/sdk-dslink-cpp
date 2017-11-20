#include "dsa_common.h"

#include "link_strand.h"

#include <boost/asio/strand.hpp>

#include "crypto/ecdh.h"

#include "module/security_manager.h"
#include "responder/node_state_manager.h"
#include "session_manager.h"

namespace dsa {
LinkStrand::LinkStrand(void* strand, ECDH* ecdh)
    : __strand(strand), __ecdh(ecdh) {}
LinkStrand::~LinkStrand() {
  if(__strand != nullptr) {
    delete static_cast<boost::asio::io_service::strand*>(__strand);
    __strand = nullptr;
  }
}

void LinkStrand::destroy_impl() {
  if (__security_manager != nullptr) {
    __security_manager->destroy();
  }
  if (__session_manager != nullptr) {
    __session_manager->destroy();
  }
  if (__stream_acceptor != nullptr) {
    __stream_acceptor->destroy();
  }
}

boost::asio::io_service& LinkStrand::get_io_service() {
  return static_cast<boost::asio::io_service::strand*>(__strand)
      ->get_io_service();
}
void LinkStrand::post(std::function<void()>&& callback) {
  static_cast<boost::asio::io_service::strand*>(__strand)->post(callback);
}

void LinkStrand::dispatch(std::function<void()>&& callback) {
  static_cast<boost::asio::io_service::strand*>(__strand)->dispatch(callback);
}
}
