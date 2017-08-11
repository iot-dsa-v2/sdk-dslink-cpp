#ifndef DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
#define DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_

#include <stdexcept>

#include <valarray>

#include "enable_closable.h"

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

  template <typename F>
  intrusive_this_lambda<T, F> make_intrusive_this_lambda(F&& func) {
    return intrusive_this_lambda<T, F>(static_cast<T*>(this)->intrusive_this(),
                                       std::forward<F>(func));
  }

  template <typename F>
  intrusive_this_lambda<const T, F> make_intrusive_this_lambda(F&& func) const {
    return intrusive_this_lambda<const T, F>(
        static_cast<T*>(this)->intrusive_this(), std::forward<F>(func));
  }

  template <typename _Ty>
  friend void intrusive_ptr_add_ref(_Ty* t);
  template <typename _Ty>
  friend void intrusive_ptr_release(_Ty* t);

  unsigned int _refs{0};

 public:
  unsigned int ref_count() const { return _refs; }
};

template <typename T>
class IntrusiveClosable : public Closable, public EnableIntrusive<T> {};

template <typename _Ty>
inline void intrusive_ptr_add_ref(_Ty* t) {
  ++t->_refs;
}

template <typename _Ty>
inline void intrusive_ptr_release(_Ty* t) {
  if (--t->_refs == 0) delete t;
}

template <class _Ty, class... _Types>
inline intrusive_ptr_<_Ty> make_intrusive_(_Types&&... _Args) {
  return intrusive_ptr_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
