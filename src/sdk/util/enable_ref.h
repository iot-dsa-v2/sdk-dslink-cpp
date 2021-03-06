#ifndef DSA_SDK_UTIL_ENABLE_INTRUSIVE_H
#define DSA_SDK_UTIL_ENABLE_INTRUSIVE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/config.hpp>

#include <boost/assert.hpp>
#include <boost/detail/workaround.hpp>
#include <functional>
#include <iosfwd>

#ifdef _DSA_DEBUG
#include <mutex>

#ifdef __APPLE__
#include <openssl/rand.h>
#endif

extern thread_local int _dsa_ref_guard_count;
extern thread_local int _dsa_ref_guard_rand;

class DsaRefGuard {
 public:
  bool released = false;
  DsaRefGuard() {
    _dsa_ref_guard_rand = rand();
    ++_dsa_ref_guard_count;
  }
  ~DsaRefGuard() { release(); }
  void release() {
    if (released) return;
    released = true;
    --_dsa_ref_guard_count;
    _dsa_ref_guard_rand = rand();
  }
};
// when DsaRefGuard is locked, refcount change shouldn't happen
#define DSA_REF_GUARD DsaRefGuard dsa_ref_guard;
#define DSA_REF_GUARD_RELEASE dsa_ref_guard.release();
// the above ref count protection can not be done on visual studio because of this bug
// https://developercommunity.visualstudio.com/content/problem/224120/lambda-can-not-be-added-into-vector-without-copy.html

#else
// do nothing in release mode
#define DSA_REF_GUARD
#define DSA_REF_GUARD_RELEASE

#endif

namespace dsa {

template <class T>
class ref_ {
 public:
  typedef T element_type;

 private:
  typedef ref_ this_type;

  T *px;

 public:
  BOOST_CONSTEXPR ref_() BOOST_NOEXCEPT : px(0) {}

#ifdef _DSA_DEBUG
  void inc_ref() {
    if (px->_refs == 0) {
      // allow a ref to be created in different thread
      px->_first_guard = _dsa_ref_guard_rand;
    } else {
      BOOST_ASSERT(_dsa_ref_guard_count == 0 ||
                   px->_first_guard == _dsa_ref_guard_rand);
    }

    BOOST_ASSERT(px->_ref_mutex.try_lock());
    ++px->_refs;
    px->_ref_mutex.unlock();
  }
  size_t dec_ref() {
    BOOST_ASSERT(_dsa_ref_guard_count == 0 ||
                 px->_first_guard == _dsa_ref_guard_rand);
    BOOST_ASSERT(px->_ref_mutex.try_lock());
    --px->_refs;
    px->_ref_mutex.unlock();
    return px->_refs;
  }
#else
  inline void inc_ref() { ++px->_refs; }
  inline size_t dec_ref() { return --px->_refs; }
#endif

  ref_(T *p) : px(p) {
    if (px != 0) inc_ref();
  }

  template <class U>
  ref_(ref_<U> const &rhs) : px(static_cast<T *>(rhs.get())) {
    if (px != 0) inc_ref();
  }

  ref_(ref_ const &rhs) : px(rhs.px) {
    if (px != 0) inc_ref();
    ;
  }

  ~ref_() {
    if (px != 0 && dec_ref() == 0) {
      delete px;
    }
  }

  template <class U>
  ref_ &operator=(ref_<U> const &rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  // Move support

  ref_(ref_ &&rhs) BOOST_NOEXCEPT : px(rhs.px) { rhs.px = 0; }

  ref_ &operator=(ref_ &&rhs) BOOST_NOEXCEPT {
    this_type(static_cast<ref_ &&>(rhs)).swap(*this);
    return *this;
  }

  template <class U>
  friend class ref_;

  template <class U>
  ref_(ref_<U> &&rhs) : px(DOWN_CAST<T *>(rhs.px)) {
    rhs.px = 0;
  }

  template <class U>
  ref_ &operator=(ref_<U> &&rhs) BOOST_NOEXCEPT {
    this_type(DOWN_CAST<ref_<U> &&>(rhs)).swap(*this);
    return *this;
  }

  ref_ &operator=(ref_ const &rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  ref_ &operator=(T *rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  void reset() BOOST_NOEXCEPT { this_type().swap(*this); }

  void reset(T *rhs) { this_type(rhs).swap(*this); }

  T *get() const BOOST_NOEXCEPT { return px; }

  T *detach() BOOST_NOEXCEPT {
    T *ret = px;
    px = 0;
    return ret;
  }

  T &operator*() const {
    BOOST_ASSERT(px != 0);
    return *px;
  }

  T *operator->() const {
    BOOST_ASSERT(px != 0);
    return px;
  }

  explicit operator bool() const BOOST_NOEXCEPT { return px != 0; }

  // operator! is redundant, but some compilers need it
  bool operator!() const BOOST_NOEXCEPT { return px == 0; }

  void swap(ref_ &rhs) BOOST_NOEXCEPT {
    T *tmp = px;
    px = rhs.px;
    rhs.px = tmp;
  }
};

// a reference that allow to keep a const global ref without multi-thread check
template <class T>
class static_ref_ {
 private:
  T *px;

 public:
  static_ref_(T *p) : px(p) { px->_refs = 0x6FFF; }
  // multi-threading might break the ref count
  // we don't care about it and just give it enough buffer to mess with
  ~static_ref_() { delete px; }

  T *get() const BOOST_NOEXCEPT { return px; }

  // cast to ref_<T>
  operator ref_<T>() {
    if (px->_refs < 0x6FFF) px->_refs = 0x6FFF;
    return ref_<T>(px);
  }
};

template <class T>
struct RefHash {
  size_t operator()(const ref_<T> &ref) const {
    return reinterpret_cast<size_t>(ref.get());
  }
  size_t operator()(const ref_<const T> &ref) const {
    return reinterpret_cast<size_t>(ref.get());
  }
};

template <class T, class U>
inline bool operator==(ref_<T> const &a, ref_<U> const &b) {
  return a.get() == b.get();
}

template <class T, class U>
inline bool operator!=(ref_<T> const &a, ref_<U> const &b) {
  return a.get() != b.get();
}

template <class T, class U>
inline bool operator==(ref_<T> const &a, U *b) {
  return a.get() == b;
}

template <class T, class U>
inline bool operator!=(ref_<T> const &a, U *b) {
  return a.get() != b;
}

template <class T, class U>
inline bool operator==(T *a, ref_<U> const &b) {
  return a == b.get();
}

template <class T, class U>
inline bool operator!=(T *a, ref_<U> const &b) {
  return a != b.get();
}

template <class T, class U>
inline bool operator==(ref_<T> const &a, static_ref_<U> const &b) {
  return a.get() == b.get();
}

template <class T, class U>
inline bool operator!=(ref_<T> const &a, static_ref_<U> const &b) {
  return a.get() != b.get();
}

template <class T>
inline bool operator==(ref_<T> const &p, std::nullptr_t) BOOST_NOEXCEPT {
  return p.get() == 0;
}

template <class T>
inline bool operator==(std::nullptr_t, ref_<T> const &p) BOOST_NOEXCEPT {
  return p.get() == 0;
}

template <class T>
inline bool operator!=(ref_<T> const &p, std::nullptr_t) BOOST_NOEXCEPT {
  return p.get() != 0;
}

template <class T>
inline bool operator!=(std::nullptr_t, ref_<T> const &p) BOOST_NOEXCEPT {
  return p.get() != 0;
}

template <class T>
inline bool operator<(ref_<T> const &a, ref_<T> const &b) {
  return std::less<T *>()(a.get(), b.get());
}

template <class T>
void swap(ref_<T> &lhs, ref_<T> &rhs) {
  lhs.swap(rhs);
}

// mem_fn support

template <class T>
T *get_pointer(ref_<T> const &p) {
  return p.get();
}

template <class T, class U>
ref_<T> static_pointer_cast(ref_<U> const &p) {
  return static_cast<T *>(p.get());
}

template <class T, class U>
ref_<T> const_pointer_cast(ref_<U> const &p) {
  return const_cast<T *>(p.get());
}

template <class T, class U>
ref_<T> dynamic_pointer_cast(ref_<U> const &p) {
  return dynamic_cast<T *>(p.get());
}

// operator<<

template <class Y>
std::ostream &operator<<(std::ostream &os, ref_<Y> const &p) {
  os << p.get();
  return os;
}

template <class T>
class EnableRef {
  friend class ref_<T>;

 public:
  ref_<T> get_ref() { return ref_<T>(static_cast<T *>(this)); }
  ref_<const T> get_ref() const {
    return ref_<const T>(static_cast<const T *>(this));
  }

#ifdef _DSA_DEBUG
  // check if ref is changed by multiple thread at same time
  mutable std::mutex _ref_mutex;
  // if ref is created in a different thread, it's ok to have some ref change
  // before the guard changes in the same thread
  mutable int _first_guard{0};
#endif
  mutable size_t _refs{0};

  EnableRef<T>(){};
  EnableRef<T>(const EnableRef<T> &rhs){};

 public:
  size_t ref_count() const { return _refs; }
};

template <typename T>
class DestroyableRef : public EnableRef<T> {
 private:
  bool _destroyed = false;

 protected:
  virtual void destroy_impl(){};

 public:
  bool is_destroyed() const { return _destroyed; }
  void destroy() {
    if (!_destroyed) {
      _destroyed = true;
      destroy_impl();
    }
  }
};

template <class _Ty, class... _Types>
ref_<_Ty> make_ref_(_Types &&... _Args) {
  return ref_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
};

template <class _Ty, class... _Types>
auto make_base_ref_(_Types &&... _Args)
    -> decltype((new _Ty(std::forward<_Types>(_Args)...))->get_ref()) {
  return (new _Ty(std::forward<_Types>(_Args)...))->get_ref();
};

template <class T>
ref_<T> copy_ref_(ref_<T> ref) {
  return ref_<T>(ref.get());
}

template <typename T>
inline ref_<T> remove_ref_(ref_<T> &p) {
  ref_<T> temp = std::move(p);
  return temp;
}

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_INTRUSIVE_H
