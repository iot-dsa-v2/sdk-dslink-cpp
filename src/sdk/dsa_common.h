#ifndef DSA_DSA_COMMON_H
#define DSA_DSA_COMMON_H

#if defined(_MSC_VER)
#pragma once

// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#ifdef _DSA_DEBUG

#define DOWN_CAST dynamic_cast
#define BOOST_ENABLE_ASSERT_HANDLER

const bool DSA_DEBUG = true;

#else  // DSA_DEBUG

#define DOWN_CAST static_cast
#define BOOST_DISABLE_ASSERTS

const bool DSA_DEBUG = false;

#endif  // _DSA_DEBUG

#include <cstdint>
#include <string>

namespace std {
#ifndef __APPLE__
template <typename T>
class shared_ptr;
#else
#include <memory>
#endif
class runtime_error;
}

namespace dsa {

const uint8_t DSA_MAJOR_VERSION = 2;
const uint8_t DSA_MINOR_VERSION = 0;

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

using string_ = std::string;

#if defined(_WIN32) || defined(_WIN64)
using wstring_ = std::wstring;
static constexpr wchar_t      w_separator = L'/';
static constexpr wchar_t      w_preferred_separator = L'\\';
static constexpr wchar_t      w_dot = L'.';
static constexpr wchar_t	  empty_wstring[] = L"";
# else 
using wstring_ = std::string;
static constexpr char      w_separator = '/';
static constexpr char      w_preferred_separator = '/';
static constexpr char      w_dot = '.';
static constexpr char	   empty_wstring[] = "";
# endif


template <typename T>
class ref_;

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H
