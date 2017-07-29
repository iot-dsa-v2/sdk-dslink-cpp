#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#include <stdexcept>

namespace dsa {

template <class T>
class EnableShared {
 private:
  std::shared_ptr<T> _ptr;

 public:
  EnableShared() : _ptr(static_cast<T*>(this)) {}

  std::shared_ptr<T> shared_from_this() {
    if (_ptr != nullptr) {
      return _ptr;
    }
    throw std::runtime_error("shared object used after destroy");
  }

  virtual void destroy() { _ptr.reset(); }

  bool destroyed() const { return !_ptr.get(); }
};

class MultipleInheritableEnableSharedFromThis : public virtual std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis> {
 public:
  virtual ~MultipleInheritableEnableSharedFromThis() {}
};

template<typename T, typename F>
class shared_this_lambda {
  std::shared_ptr<T> t;  // just for lifetime
  F f;
 public:
  shared_this_lambda(std::shared_ptr<T> t, F f) : t(t), f(f) {}
  template<class... Args>
  auto operator()(Args &&...args) -> decltype(this->f(std::forward<Args>(args)...)) {
    return f(std::forward<Args>(args)...);
  }
};

template <class T>
class InheritableEnableShared : virtual public MultipleInheritableEnableSharedFromThis {
 public:
  std::shared_ptr<T> shared_from_this() {
    return std::dynamic_pointer_cast<T>(MultipleInheritableEnableSharedFromThis::shared_from_this());
  }

  template <class Down>
  std::shared_ptr<Down> share_this() {
    return std::dynamic_pointer_cast<Down>(MultipleInheritableEnableSharedFromThis::shared_from_this());
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
};

template <class _Ty, class... _Types>
inline std::shared_ptr<_Ty> make_shared(_Types&&... _Args) {
  // FIXME: this doesn't compile
//    return (new _Ty(_STD, std::forward<_Types>(_Args)...))->shared_from_this();
  return (new _Ty(_Args...))->shared_from_this();
}

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_