#ifndef DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
#define DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_

#include <atomic>

#include <dsa_common.h>

#include <stdexcept>

#include <valarray>

namespace dsa {

template <typename T, typename F>
class intrusive_this_lambda {
 private:
  intrusive_ptr_<T> _this;
  F _func;

 public:
  intrusive_this_lambda(intrusive_ptr_<T> t, F f) : _this(t), _func(f) {}
  template <typename... _Args>
  auto operator()(_Args&&... args)
      -> decltype(this->_func(std::forward<_Args>(args)...)) {
    return _func(std::forward<_Args>(args)...);
  }
};

template <class T>
class EnableIntrusive {
 protected:
  intrusive_ptr_<T> intrusive_this() {
    return intrusive_ptr_<T>(static_cast<T*>(this));
  }

  template <typename _Downcast>
  intrusive_ptr_<_Downcast> intrusive_this() {
    return intrusive_ptr_<_Downcast>(dynamic_cast<_Downcast*>(this));
  }

  template <typename _Ty>
  friend void intrusive_ptr_add_ref(_Ty* t);
  template <typename _Ty>
  friend void intrusive_ptr_release(_Ty* t);

  size_t _refs{0};

 public:
  size_t ref_count() const { return _refs; }
};

template <typename T>
class IntrusiveClosable : public EnableIntrusive<T> {
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
intrusive_ptr_<_Ty> make_intrusive_(_Types&&... _Args) {
  return intrusive_ptr_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
