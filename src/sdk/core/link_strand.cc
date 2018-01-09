#include "dsa_common.h"

#include "link_strand.h"

#include <boost/asio/strand.hpp>

#include "crypto/ecdh.h"
#include "module/security_manager.h"
#include "module/session_manager.h"
#include "responder/node_state_manager.h"
#include "strand_timer.h"

namespace dsa {

LinkStrand::LinkStrand(void* strand, ECDH* ecdh)
    : __strand(strand), __ecdh(ecdh) {}
LinkStrand::~LinkStrand() {
  if (__strand != nullptr) {
    delete static_cast<boost::asio::io_context::strand*>(__strand);
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

boost::asio::io_context& LinkStrand::get_io_context() {
  return static_cast<boost::asio::io_context::strand*>(__strand)
      ->get_io_context();
}
void LinkStrand::post(std::function<void()>&& callback) {
  static_cast<boost::asio::io_context::strand*>(__strand)->post(
      std::move(callback));
}

void LinkStrand::dispatch(std::function<void()>&& callback) {
  static_cast<boost::asio::io_context::strand*>(__strand)->dispatch(
      std::move(callback));
}

ref_<StrandTimer> LinkStrand::add_timer(int32_t interval_ms,
                                        TimerCallback&& callback) {
  return ref_<StrandTimer>(
      new StrandTimer(get_ref(), interval_ms, std::move(callback)));
}
}
