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

#include <cstdint>
#include <string>

namespace std {
template <typename T>
class shared_ptr;
class runtime_error;
}

namespace dsa {

const uint8_t DSA_MAJOR_VERSION = 2;
const uint8_t DSA_MINOR_VERSION = 0;

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

using string_ = std::string;

template <typename T>
class ref_;

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
