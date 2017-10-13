#include "dsa_common.h"

#include "buffer.h"

#include <sstream>

std::ostream &operator<<(std::ostream &os, const dsa::IntrusiveBytes &buf) {
  std::stringstream ss;
  ss << "[";
  if (!buf.empty()) {
    for (unsigned int i = 0; i < buf.size() - 1; ++i) {
      ss << std::hex << (unsigned int) (buf[i]) << std::dec << ", ";
    }
    ss << std::hex << (unsigned int) (buf[buf.size() - 1]) << std::dec;
  }
  ss << "]";
  return os << ss.str();
}

