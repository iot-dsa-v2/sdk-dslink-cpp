#include "dsa_common.h"

#include "link_strand.h"

#include <boost/asio/strand.hpp>

#include "crypto/ecdh.h"

namespace dsa {
LinkStrand::LinkStrand(void* strand, ECDH* ecdh)
    : __strand(strand), __ecdh(ecdh) {}
LinkStrand::~LinkStrand() {
  delete static_cast<boost::asio::io_service::strand*>(__strand);
  delete __ecdh;
}
boost::asio::io_service &LinkStrand::get_io_service(){
  static_cast<boost::asio::io_service::strand*>(__strand)->get_io_service();
}
void LinkStrand::post(std::function<void()>&& callback) {
  static_cast<boost::asio::io_service::strand*>(__strand)->post(callback);
}

void LinkStrand::dispatch(std::function<void()>&& callback) {
  static_cast<boost::asio::io_service::strand*>(__strand)->dispatch(callback);
}

}
