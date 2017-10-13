#ifndef DSA_SDK_UTIL_BUFFER_H_
#define DSA_SDK_UTIL_BUFFER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include "enable_intrusive.h"

namespace dsa {
class Message;

class IntrusiveBytes : public std::vector<uint8_t>,
                       public EnableRef<IntrusiveBytes> {
 public:
  template <typename... Args>
  inline explicit IntrusiveBytes(Args &&... args)
      : std::vector<uint8_t>(std::forward<Args>(args)...){};

  typedef std::vector<uint8_t>::iterator iterator;
  typedef std::vector<uint8_t>::const_iterator const_iterator;
};

typedef ref_<const IntrusiveBytes> BytesRef;
}  // namespace dsa

std::ostream &operator<<(std::ostream &os, const dsa::IntrusiveBytes &buf);

#endif  // DSA_SDK_UTIL_BUFFER_H_
