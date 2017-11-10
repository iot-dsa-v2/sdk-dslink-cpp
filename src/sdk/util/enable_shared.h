#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>
#include <mutex>
#include <memory>

namespace dsa {

template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args &&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
class SharedDestroyable : public std::enable_shared_from_this<T> {
 private:
  bool _destroyed = false;
  bool _pending_destroy = false;

 protected:
  virtual void destroy_impl(){};
  virtual void post_in_strand(std::function<void()> &&) = 0;

 public:
  shared_ptr_<T> shared_from_this() {
    return std::enable_shared_from_this<T>::shared_from_this();
  }

  template <class Down>
  shared_ptr_<Down> share_this() {
    return std::dynamic_pointer_cast<Down>(shared_from_this());
  }

  std::mutex mutex;
  bool is_destroyed() const { return _destroyed; }

  // reuse any lock
  void destroy(std::lock_guard<std::mutex> &unique_lock) {
    if (!_destroyed || _pending_destroy) {
      _destroyed = true;
      _pending_destroy = false;
      destroy_impl();
    }
  }
  void destroy() {
    std::lock_guard<std::mutex> unique_lock(mutex);
    destroy(unique_lock);
  }

  void destroy_in_strand(shared_ptr_<SharedDestroyable<T>> &&destroyable) {
    if (_destroyed) return;
    _pending_destroy = true;
    _destroyed = true;
    post_in_strand(
        [ this, keep_ptr = std::move(destroyable) ]() { destroy(); });
  }
  void destroy_in_strand(shared_ptr_<SharedDestroyable<T>> &destroyable) {
    post_in_strand([ this, keep_ptr = destroyable ]() { destroy(); });
  }
};
}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_
