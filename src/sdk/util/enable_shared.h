#ifndef DSA_SDK_UTIL_ENABLE_SHARED_H_
#define DSA_SDK_UTIL_ENABLE_SHARED_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>

#include <memory>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace dsa {

template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args &&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}


template <class T>
class EnableShared : public std::enable_shared_from_this<T> {
 public:
  shared_ptr_<T> shared_from_this() {
    return std::enable_shared_from_this<T>::shared_from_this();
  }

  template <class Down>
  shared_ptr_<Down> share_this() {
    return std::dynamic_pointer_cast<Down>(shared_from_this());
  }
};

template <typename T>
class SharedDestroyable : public EnableShared<T> {
 private:
  bool _destroyed = false;

 protected:
  virtual void destroy_impl(){};
  virtual void dispatch_in_strand(std::function<void()> &&) = 0;

 public:
  boost::shared_mutex mutex;
  bool is_destroyed() const { return _destroyed; }

  // reuse any lock
  void destroy(boost::unique_lock<boost::shared_mutex> &unique_lock) {
    if (!_destroyed) {
      _destroyed = true;
      destroy_impl();
    }
  }
  void destroy() {
    boost::unique_lock<boost::shared_mutex> unique_lock(mutex);
    destroy(unique_lock);
  }

  static void destroy_in_strand(shared_ptr_ <SharedDestroyable<T>> destroyable) {
    SharedDestroyable<T> *raw_ptr = destroyable.get();
    // obtain the lock before dispatch to strand to reduce the load on main
    // strand
    raw_ptr->dispatch_in_strand([
      destroyable = std::move(destroyable)
    ]() mutable { destroyable->destroy(); });
  }
};
}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_SHARED_H_
