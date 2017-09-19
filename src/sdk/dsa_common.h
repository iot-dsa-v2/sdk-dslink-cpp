#ifndef DSA_DSA_COMMON_H_
#define DSA_DSA_COMMON_H_

#if defined(_MSC_VER)
#pragma once


// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#ifdef DSA_DEBUG

#define DOWN_CAST dynamic_cast
#define BOOST_ENABLE_ASSERT_HANDLER

#else  // DSA_DEBUG

#define DOWN_CAST static_cast
#define BOOST_DISABLE_ASSERTS

#endif  // DSA_DEBUG

#include <boost/intrusive_ptr.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <typename T>
using ref_ = boost::intrusive_ptr<T>;

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
