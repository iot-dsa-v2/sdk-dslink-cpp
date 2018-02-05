#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>
#include <mutex>
#include <memory>
#include <functional>

namespace dsa {

template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args &&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
class SharedDestroyable : public std::enable_shared_from_this<T> {
 private:
  bool _destroyed = false;

 protected:
  virtual void destroy_impl(){};

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
    if (!_destroyed) {
      _destroyed = true;
      destroy_impl();
    }
  }
  void destroy() {
    std::lock_guard<std::mutex> unique_lock(mutex);
    destroy(unique_lock);
  }
};

template <typename T>
class SharedStrandPtr : public SharedDestroyable<T> {
 private:

 protected:
  virtual void post_in_strand(std::function<void()> &&) = 0;

 public:
  void destroy_in_strand(shared_ptr_<SharedStrandPtr<T>> &&destroyable) {
    post_in_strand(
        [ this, keep_ptr = std::move(destroyable) ]() { this->destroy(); });
  }
  void destroy_in_strand(shared_ptr_<SharedStrandPtr<T>> &destroyable) {
    post_in_strand([ this, keep_ptr = destroyable ]() { this->destroy(); });
  }
};
}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_
