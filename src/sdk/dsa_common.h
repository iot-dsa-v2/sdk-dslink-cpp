#ifndef DSA_DSA_COMMON_H_
#define DSA_DSA_COMMON_H_

#if _MSC_VER

// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <boost/intrusive_ptr.hpp>

namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <typename T>
using intrusive_ptr_ = boost::intrusive_ptr<T>;

}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
