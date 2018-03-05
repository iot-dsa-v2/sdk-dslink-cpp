#ifndef DSA_SDK_FIELDS_ALLOC_H
#define DSA_SDK_FIELDS_ALLOC_H

#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <memory>
#include <stdexcept>

namespace dsa {

namespace detail {

struct static_pool {
  std::size_t size_;
  std::size_t refs_ = 1;
  std::size_t count_ = 0;
  char* p_;

  char* end() { return reinterpret_cast<char*>(this + 1) + size_; }

  explicit static_pool(std::size_t size)
      : size_(size), p_(reinterpret_cast<char*>(this + 1)) {}

 public:
  static static_pool& construct(std::size_t size) {
    auto p = new char[sizeof(static_pool) + size];
    return *(new (p) static_pool{size});
  }

  static_pool& share() {
    ++refs_;
    return *this;
  }

  void destroy() {
    if (refs_--) return;
    this->~static_pool();
    delete[] reinterpret_cast<char*>(this);
  }

  void* alloc(std::size_t n) {
    auto last = p_ + n;
    if (last >= end()) BOOST_THROW_EXCEPTION(std::bad_alloc{});
    ++count_;
    auto p = p_;
    p_ = last;
    return p;
  }

  void dealloc() {
    if (--count_) return;
    p_ = reinterpret_cast<char*>(this + 1);
  }
};

}  // detail

template <class T>
struct fields_alloc {
  detail::static_pool& pool_;

 public:
  using value_type = T;
  using is_always_equal = std::false_type;
  using pointer = T*;
  using reference = T&;
  using const_pointer = T const*;
  using const_reference = T const&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <class U>
  struct rebind {
    using other = fields_alloc<U>;
  };

  explicit fields_alloc(std::size_t size)
      : pool_(detail::static_pool::construct(size)) {}

  fields_alloc(fields_alloc const& other) : pool_(other.pool_.share()) {}

  template <class U>
  fields_alloc(fields_alloc<U> const& other) : pool_(other.pool_.share()) {}

  ~fields_alloc() { pool_.destroy(); }

  value_type* allocate(size_type n) {
    return static_cast<value_type*>(pool_.alloc(n * sizeof(T)));
  }

  void deallocate(value_type*, size_type) { pool_.dealloc(); }

#if defined(BOOST_LIBSTDCXX_VERSION) && BOOST_LIBSTDCXX_VERSION < 60000
  template <class U, class... Args>
  void construct(U* ptr, Args&&... args) {
    ::new ((void*)ptr) U(std::forward<Args>(args)...);
  }

  template <class U>
  void destroy(U* ptr) {
    ptr->~U();
  }
#endif

  template <class U>
  friend bool operator==(fields_alloc const& lhs, fields_alloc<U> const& rhs) {
    return &lhs.pool_ == &rhs.pool_;
  }

  template <class U>
  friend bool operator!=(fields_alloc const& lhs, fields_alloc<U> const& rhs) {
    return !(lhs == rhs);
  }
};
}  // dsa

#endif  // DSA_SDK_FIELDS_ALLOC_H
