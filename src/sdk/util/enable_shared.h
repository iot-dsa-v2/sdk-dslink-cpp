#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#include <stdexcept>

#include <boost/intrusive_ptr.hpp>
#include <valarray>

namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

//class MultipleInheritableEnableSharedFromThis : public virtual std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis> {
// public:
//  virtual ~MultipleInheritableEnableSharedFromThis() {}
//};

template<typename T, typename F>
class shared_this_lambda {
  shared_ptr_<T> t;  // just for lifetime
  F f;
 public:
  shared_this_lambda(shared_ptr_<T> t, F f) : t(t), f(f) {}
  template<class... Args>
  auto operator()(Args &&...args) -> decltype(this->f(std::forward<Args>(args)...)) {
    return f(std::forward<Args>(args)...);
  }
};

template <class T>
class GracefullyClosable : public std::enable_shared_from_this<GracefullyClosable<T>> {
//class GracefullyClosable : virtual public MultipleInheritableEnableSharedFromThis {
 public:
  shared_ptr_<T> shared_from_this() {
    return std::dynamic_pointer_cast<T>(std::enable_shared_from_this<GracefullyClosable<T>>::shared_from_this());
  }

  template <class Down>
  shared_ptr_<Down> share_this() {
    return std::dynamic_pointer_cast<Down>(shared_from_this());
  }

  template<typename F>
  auto make_shared_this_lambda(F f) -> shared_this_lambda<T, F> {
    return shared_this_lambda<T, F>(
        static_cast<T *>(this)->shared_from_this(), f);
  }

  template<typename F>
  auto make_shared_this_lambda(F f) const -> shared_this_lambda<const T, F> {
    return shared_this_lambda<const T, F>(
        static_cast<const T *>(this)->shared_from_this(), f);
  }

  virtual void close() = 0;
};

template <typename T>
using intrusive_ptr_ = boost::intrusive_ptr<T>;

template <typename T, typename F>
class intrusive_this_lambda {
 private:
  intrusive_ptr_<T> _this;
  F _func;
 public:
  intrusive_this_lambda(intrusive_ptr_<T> t, F f) : _this(t), _func(f) {}
  template <typename... _Args>
  auto operator()(_Args&&... args) -> decltype(this->_func(std::forward<_Args>(args)...)) {
    return _func(std::forward<_Args>(args)...);
  }
};

template <class T>
class EnableIntrusive {
 protected:
  intrusive_ptr_<T> intrusive_this() { return intrusive_ptr_<T>(static_cast<T*>(this)); }

  template <typename F>
  intrusive_this_lambda<T, F> make_intrusive_this_lambda(F&& func) {
    return intrusive_this_lambda<T, F>(static_cast<T*>(this)->intrusive_this(), std::forward<F>(func));
  }

  template <typename F>
  intrusive_this_lambda<const T, F> make_intrusive_this_lambda(F&& func) const {
    return intrusive_this_lambda<const T, F>(static_cast<T*>(this)->intrusive_this(), std::forward<F>(func));
  }

  template <typename _Ty>
  friend void intrusive_ptr_add_ref(_Ty* t);
  template <typename _Ty>
  friend void intrusive_ptr_release(_Ty* t);

  unsigned int _refs{0};

 public:
  unsigned int ref_count() const { return _refs; }
};

//class MultipleInheritableEnableIntrusive : public EnableIntrusive<MultipleInheritableEnableIntrusive> {
//public:
//  virtual ~MultipleInheritableEnableIntrusive() = default;
//};
//
//template <typename T>
//class InheritableEnableIntrusive : virtual public MultipleInheritableEnableIntrusive {
//protected:
//  intrusive_ptr_<T> intrusive_this() { return intrusive_ptr_<T>(static_cast<T*>(this)); }
//
//  template <typename _Down>
//  intrusive_ptr_<_Down> intrusive_this() { return intrusive_ptr_<_Down>(static_cast<_Down*>(this)); }
//};

template <typename _Ty>
inline void intrusive_ptr_add_ref(_Ty* t){
  ++t->_refs;
}

template <typename _Ty>
inline void intrusive_ptr_release(_Ty* t){
  if(--t->_refs == 0)
    delete t;
}

//template <class _Ty, class... _Types>
//inline typename std::enable_if <
//    std::is_base_of<EnableIntrusive<_Ty>, _Ty>::value,
//    intrusive_ptr_<_Ty>
//>::type make_intrusive_(_Types&&... _Args) {
//  return intrusive_ptr_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
//}

template <class _Ty, class... _Types>
inline intrusive_ptr_<_Ty> make_intrusive_(_Types&&... _Args) {
  return intrusive_ptr_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_