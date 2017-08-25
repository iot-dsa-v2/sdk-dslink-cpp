#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#include <stdexcept>

#include <valarray>

#include <atomic>

namespace dsa {



template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename F>
class shared_this_lambda {
  shared_ptr_<T> t;  // just for lifetime
  F f;

 public:
  shared_this_lambda(shared_ptr_<T> t, F f) : t(t), f(f) {}
  template <class... Args>
  auto operator()(Args&&... args)
      -> decltype(this->f(std::forward<Args>(args)...)) {
    return f(std::forward<Args>(args)...);
  }
};

template <class T>
class EnableShared : public std::enable_shared_from_this<T> {
 public:
  shared_ptr_<T> shared_from_this() {
    return std::enable_shared_from_this<T>::shared_from_this();
  }

  template <class Down>
  shared_ptr_<Down> share_this() {
    return std::static_pointer_cast<Down>(std::move(shared_from_this()));
  }
};

template <typename T>
class SharedClosable : public EnableShared<T> {
private:
  std::atomic_bool _closed{false};

public:
  bool is_closed() const { return _closed; }
  void close() {
    if (!_closed) {
      _closed = true;
      close_impl();
    }
  }
  virtual void close_impl(){};
};
}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_
