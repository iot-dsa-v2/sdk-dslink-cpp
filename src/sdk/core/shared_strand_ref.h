#ifndef DSA_SDK_SHARED_REF_H__
#define DSA_SDK_SHARED_REF_H__

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <memory>
#include "link_strand.h"

namespace dsa {

// a shared_ptr wrapper on ref_ to make it easier to use ref_ in other thread
template <class T>
class SharedRef : public std::enable_shared_from_this<SharedRef<T>> {
  ref_<LinkStrand> _strand;
  ref_<T> _ref;

 public:
  typedef std::function<void(T&, LinkStrand&)> PostCallback;

  static shared_ptr_<SharedRef<T>> make(ref_<T> ref,
                                        const ref_<LinkStrand>& strand) {
    return std::make_shared<SharedRef<T>>(std::move(ref), strand);
  }

  SharedRef(ref_<T> ref, const ref_<LinkStrand>& strand)
      : _strand(strand), _ref(std::move(ref)) {}

  ~SharedRef() {
    auto p_strand = _strand.get();
    // post 2 reference to strand and they will be released there
    p_strand->post([ strand = std::move(_strand), ref = std::move(_ref) ](){});
  }

  SharedRef(const SharedRef&) = delete;
  SharedRef& operator=(const SharedRef&) = delete;

  void post(PostCallback&& callback) {
    _strand->post([
      this, keep_shared = this->shared_from_this(),
      callback = std::move(callback)
    ]() { callback(*_ref, *_strand); });
  }
};
}

#define POST_TO_REF(shared_ref, function_name) \
  shared_ref->post([](auto& t, LinkStrand& strand) { t.function_name(); });

// one parameter as R reference
#define POST_TO_REF_R(shared_ref, function_name, v1)                     \
  shared_ref->post([cv1 = std::move(v1)](auto& t, LinkStrand& strand) { \
    t.function_name(std::move(cv1));                                    \
  });


#endif  // DSA_SDK_SHARED_REF_H__
