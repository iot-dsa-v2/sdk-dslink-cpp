#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#include <memory>
#include <stdexcept>

namespace dsa {

template <class T>
class EnableShared {
 private:
  std::shared_ptr<T> _sptr;

 public:
  Enabled_Shared() { _sptr = shared_ptr<T>(this); }

  shared_ptr<T> shared_from_this() {
    if (_sptr.get()) {
      return _sptr;
    }
    throw std::runtime_error("shared object used after destory");
  }

  void destory() { _sptr.reset(nullptr); }

  bool destroyed() const { return !_sptr.get(); }
};

template <class _Ty, class... _Types>
inline shared_ptr<_Ty> make_shared(_Types&&... _Args) {
  return (new _Ty(_STD forward<_Types>(_Args)...))->shared_from_this();
}

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_
