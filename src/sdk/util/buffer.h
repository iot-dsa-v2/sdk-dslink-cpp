#ifndef DSA_SDK_UTIL_BUFFER_H_
#define DSA_SDK_UTIL_BUFFER_H_

#include <iostream>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
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

typedef ref_<IntrusiveBytes> BytesRef;
}  // namespace dsa

inline std::ostream &operator<<(std::ostream &os, const dsa::IntrusiveBytes &buf) {
  std::stringstream ss;
  ss << "[";
  if (buf.size() > 0) {
    for (unsigned int i = 0; i < buf.size() - 1; ++i) {
      ss << std::hex << (unsigned int) (buf[i]) << std::dec << ", ";
    }
    ss << std::hex << (unsigned int) (buf[buf.size() - 1]) << std::dec;
  }
  ss << "]";
  return os << ss.str();
}

#endif  // DSA_SDK_UTIL_BUFFER_H_
