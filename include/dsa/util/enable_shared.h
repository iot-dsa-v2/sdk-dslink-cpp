#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#include <memory>
#include <stdexcept>

namespace dsa {

template <class T>
class EnableShared {
 private:
  std::shared_ptr<T> _ptr;

 public:
  EnableShared() : _ptr(static_cast<T*>(this)) {}

  std::shared_ptr<T> shared_from_this() {
    if (_ptr.get()) {
      return _ptr;
    }
    throw std::runtime_error("shared object used after destory");
  }

  void destory() { _ptr.reset(); }

  bool destroyed() const { return !_ptr.get(); }
};

template <class _Ty, class... _Types>
inline std::shared_ptr<_Ty> make_shared(_Types&&... _Args) {
  return (new _Ty(_STD forward<_Types>(_Args)...))->shared_from_this();
}

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_
