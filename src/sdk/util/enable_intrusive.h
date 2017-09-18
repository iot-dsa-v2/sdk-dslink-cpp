#ifndef DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
#define DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <atomic>

#include <dsa_common.h>

#include <stdexcept>

#include <valarray>

namespace dsa {

template <class T>
class EnableRef {
 public:
  ref_<T> get_ref() { return ref_<T>(static_cast<T*>(this)); }
  ref_<const T> get_ref() const {
    return ref_<const T>(static_cast<const T*>(this));
  }
  //  template <typename _Downcast>
  //  ref_<_Downcast> get_ref() {
  //    return ref_<_Downcast>(static_cast<_Downcast*>(this));
  //  }

  template <typename _Ty>
  friend void intrusive_ptr_add_ref(_Ty* t);
  template <typename _Ty>
  friend void intrusive_ptr_release(_Ty* t);

  mutable size_t _refs{0};

 public:
  size_t ref_count() const { return _refs; }
};

template <typename T>
class ClosableRef : public EnableRef<T> {
 private:
  bool _closed = false;

 protected:
  virtual void close_impl(){};

 public:
  bool is_closed() const { return _closed; }
  void close() {
    if (!_closed) {
      _closed = true;
      close_impl();
    }
  }
};

template <typename _Ty>
void intrusive_ptr_add_ref(_Ty* t) {
  ++t->_refs;
}

template <typename _Ty>
void intrusive_ptr_release(_Ty* t) {
  if (--t->_refs == 0) {
    delete t;
  }
}

template <class _Ty, class... _Types>
ref_<_Ty> make_ref_(_Types&&... _Args) {
  return ref_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
};

template <class T>
ref_<T> copy_ref_(ref_<T> ref){
  return ref_<T>(ref.get());
}


template <typename T, typename TBase>
ref_<T> ref_cast_(ref_<TBase>& ref) {
  return ref_<T>(DOWN_CAST<T*>(ref.get()));
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
