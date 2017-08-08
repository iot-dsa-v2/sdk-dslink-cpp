#ifndef DSA_DSA_COMMON_H_
#define DSA_DSA_COMMON_H_

#if _MSC_VER

// disable VC warning on throw error
#pragma warning(disable : 4290)

#endif  // _MSC_VER

#include <cstdint>
#include <memory>
#include <string>

namespace dsa {

template <typename T>
using shared_ptr_ = std::shared_ptr<T>;

template <class T, typename... Args>
inline shared_ptr_<T> make_shared_(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
}  // namespace dsa

#endif  // DSA_DSA_COMMON_H_
