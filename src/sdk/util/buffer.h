#ifndef DSA_SDK_UTIL_BUFFER_H_
#define DSA_SDK_UTIL_BUFFER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include "enable_intrusive.h"

namespace dsa {
class Message;

class RefCountBytes : public std::vector<uint8_t>,
                       public EnableRef<RefCountBytes> {
 public:
  template <typename... Args>
  inline explicit RefCountBytes(Args &&... args)
      : std::vector<uint8_t>(std::forward<Args>(args)...){};

  typedef std::vector<uint8_t>::iterator iterator;
  typedef std::vector<uint8_t>::const_iterator const_iterator;
};

typedef ref_<const RefCountBytes> BytesRef;
}  // namespace dsa

std::ostream &operator<<(std::ostream &os, const dsa::RefCountBytes &buf);

#endif  // DSA_SDK_UTIL_BUFFER_H_
