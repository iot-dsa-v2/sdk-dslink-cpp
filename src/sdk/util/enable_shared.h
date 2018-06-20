#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H
#define DSA_SDK_UTIL_ENABLE_SHARED_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace dsa {

template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args &&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
inline shared_ptr_<T> remove_ptr_(shared_ptr_<T> &p) {
  shared_ptr_<T> temp = std::move(p);
  return temp;
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

  virtual ~SharedDestroyable() = default;
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
  virtual void post_in_strand(std::function<void()> &&,
                              bool locked = false) = 0;

 public:
  void destroy_in_strand(shared_ptr_<SharedStrandPtr<T>> &&destroyable,
                         bool locked = false) {
    if (destroyable->is_destroyed()) {
      return;
    }
    post_in_strand(
        [this, keep_ptr = std::move(destroyable)]() { this->destroy(); },
        locked);
  }
  void destroy_in_strand(shared_ptr_<SharedStrandPtr<T>> &destroyable,
                         bool locked = false) {
    if (destroyable->is_destroyed()) {
      return;
    }
    post_in_strand([this, keep_ptr = destroyable]() { this->destroy(); },
                   locked);
  }
};
}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H
