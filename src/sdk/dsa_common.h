#ifndef DSA_DSA_COMMON_H_
#define DSA_DSA_COMMON_H_

#if _MSC_VER

// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#define DOWN_CAST dynamic_cast

#include <algorithm>
#include <boost/intrusive_ptr.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "module/logger.h"

namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <typename T>
using ref_ = boost::intrusive_ptr<T>;

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
