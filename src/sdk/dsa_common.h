#ifndef DSA_DSA_COMMON_H_
#define DSA_DSA_COMMON_H_

#if _MSC_VER

// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#include <algorithm>
#include <boost/intrusive_ptr.hpp>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <typename T>
using intrusive_ptr_ = boost::intrusive_ptr<T>;

template <typename T>
void print(const T &t) {
#ifdef CMAKE_CXX_FLAGS_DEBUG
  std::cout << t << std::endl;
#endif
}

template <typename T1, typename T2>
void print(const T1 &t1, const T2 &t2) {
#ifdef CMAKE_CXX_FLAGS_DEBUG
  std::cout << t1 << "  " << t2 << std::endl;
#endif
}

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
