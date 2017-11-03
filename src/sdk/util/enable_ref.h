#ifndef DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
#define DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/config.hpp>

#include <boost/assert.hpp>
#include <boost/detail/workaround.hpp>

#include <functional>

#include <iosfwd>

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

  ref_(T *p) : px(p) {
    if (px != 0) ++px->_refs;
  }

  template <class U>
  ref_(ref_<U> const &rhs) : px(static_cast<T *>(rhs.get())) {
    if (px != 0) ++px->_refs;
  }

  ref_(ref_ const &rhs) : px(rhs.px) {
    if (px != 0) ++px->_refs;
    ;
  }

  ~ref_() {
    if (px != 0 && --px->_refs == 0) {
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

  mutable size_t _refs{0};

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

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
