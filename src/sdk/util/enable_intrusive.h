#ifndef DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
#define DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/config.hpp>

#include <boost/assert.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/smart_ptr/detail/sp_nullptr_t.hpp>

#include <functional>

#if !defined(BOOST_NO_IOSTREAM)
#include <ostream>
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

  ref_(T *p) : px(p) {
    if (px != 0) ++px->_refs;
  }

  template <class U>
  ref_(ref_<U> const &rhs) : px(rhs.get()) {
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
  ref_(ref_<U> &&rhs) : px(rhs.px) {
    rhs.px = 0;
  }

  template <class U>
  ref_ &operator=(ref_<U> &&rhs) BOOST_NOEXCEPT {
    this_type(static_cast<ref_<U> &&>(rhs)).swap(*this);
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
inline bool operator==(ref_<T> const &p,
                       boost::detail::sp_nullptr_t) BOOST_NOEXCEPT {
  return p.get() == 0;
}

template <class T>
inline bool operator==(boost::detail::sp_nullptr_t,
                       ref_<T> const &p) BOOST_NOEXCEPT {
  return p.get() == 0;
}

template <class T>
inline bool operator!=(ref_<T> const &p,
                       boost::detail::sp_nullptr_t) BOOST_NOEXCEPT {
  return p.get() != 0;
}

template <class T>
inline bool operator!=(boost::detail::sp_nullptr_t,
                       ref_<T> const &p) BOOST_NOEXCEPT {
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

#if !defined(BOOST_NO_IOSTREAM)

#if defined(BOOST_NO_TEMPLATED_IOSTREAMS) || \
    (defined(__GNUC__) && (__GNUC__ < 3))

template <class Y>
std::ostream &operator<<(std::ostream &os, ref_<Y> const &p) {
  os << p.get();
  return os;
}

#else

// in STLport's no-iostreams mode no iostream symbols can be used
#ifndef _STLP_NO_IOSTREAMS

#if defined(BOOST_MSVC) && \
    BOOST_WORKAROUND(BOOST_MSVC, < 1300 && __SGI_STL_PORT)
// MSVC6 has problems finding std::basic_ostream through the using declaration
// in namespace _STL
using std::basic_ostream;
template <class E, class T, class Y>
basic_ostream<E, T> &operator<<(basic_ostream<E, T> &os, ref_<Y> const &p)
#else
template <class E, class T, class Y>
std::basic_ostream<E, T> &operator<<(std::basic_ostream<E, T> &os,
                                     ref_<Y> const &p)
#endif
{
  os << p.get();
  return os;
}

#endif  // _STLP_NO_IOSTREAMS

#endif  // __GNUC__ < 3

#endif  // !defined(BOOST_NO_IOSTREAM)

template <class T>
class EnableRef {
  friend class ref_<T>;

 public:
  ref_<T> get_ref() { return ref_<T>(static_cast<T *>(this)); }
  ref_<const T> get_ref() const {
    return ref_<const T>(static_cast<const T *>(this));
  }
  //  template <typename _Downcast>
  //  ref_<_Downcast> get_ref() {
  //    return ref_<_Downcast>(static_cast<_Downcast*>(this));
  //  }

  mutable size_t _refs{0};

 public:
  size_t ref_count() const { return _refs; }
};

template <typename T>
class ClosableRef : public EnableRef<T> {
 private:
  bool _closed = false;

 protected:
  virtual void close_impl(){};

 public:
  bool is_closed() const { return _closed; }
  void close() {
    if (!_closed) {
      _closed = true;
      close_impl();
    }
  }
};

template <class _Ty, class... _Types>
ref_<_Ty> make_ref_(_Types &&... _Args) {
  return ref_<_Ty>(new _Ty(std::forward<_Types>(_Args)...));
};

template <class T>
ref_<T> copy_ref_(ref_<T> ref) {
  return ref_<T>(ref.get());
}

template <typename T, typename TBase>
ref_<T> ref_cast_(ref_<TBase> &ref) {
  return ref_<T>(DOWN_CAST<T *>(ref.get()));
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_INTRUSIVE_H_
