#ifndef DSA_SDK_SHARED_REF_H_
#define DSA_SDK_SHARED_REF_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <memory>
#include "crypto/ecdh.h"
#include "link_strand.h"

namespace dsa {

// a shared_ptr wrapper on ref_ to make it easier to use ref_ in other thread
template <class T>
class SharedRef : public std::enable_shared_from_this<SharedRef<T>> {
  LinkStrandRef _strand;
  ref_<T> _ref;

 public:
  typedef std::function<void(ref_<T>&, LinkStrand&)> PostCallback;

  static shared_ptr_<SharedRef<T>> make(const ref_<T>& ref,
                                        const LinkStrandRef& strand) {
    return std::make_shared<SharedRef<T>>(ref, strand);
  }

  SharedRef(const ref_<T>& ref, const LinkStrandRef& strand)
      : _strand(strand), _ref(ref) {}

  ~SharedRef() {
    auto p_strand = _strand.get();
    // post 2 reference to strand and they will be released there
    p_strand->dispatch(
        [ strand = std::move(_strand), ref = std::move(_ref) ](){});
  }

  SharedRef(const SharedRef&) = delete;
  SharedRef& operator=(const SharedRef&) = delete;

  void post(PostCallback&& callback) {
    _strand->post([
      this, keep_shared = this->shared_from_this(),
      callback = std::move(callback)
    ]() { callback(_ref, *_strand); });
  }

  void post(std::function<void()>&& callback) {
    _strand->post(std::move(callback));
  }

  boost::asio::io_context& get_io_context() {
    return _strand->get_io_context();
  }
  ECDH get_ecdh() { return ECDH(_strand->ecdh()); }
};

typedef shared_ptr_<SharedRef<LinkStrand>> SharedLinkStrandRef;

inline SharedLinkStrandRef share_strand_(const LinkStrandRef& strand) {
  return std::make_shared<SharedRef<LinkStrand>>(nullptr, strand);
}
}  // namespace dsa

#define POST_TO_REF(shared_ref, function_name) \
  shared_ref->post([](auto& t, LinkStrand& strand) { t->function_name(); });

// one parameter as R reference
#define POST_TO_REF_R(shared_ref, function_name, v1)                    \
  shared_ref->post([cv1 = std::move(v1)](auto& t, LinkStrand& strand) { \
    t->function_name(std::move(cv1));                                   \
  });

#endif  // DSA_SDK_SHARED_REF_H_
