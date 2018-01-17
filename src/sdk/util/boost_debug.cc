#include "dsa_common.h"

#ifdef _DSA_DEBUG

#include <iostream>
#include <sstream>

// debug handler that allows boost to throw error normally
namespace boost {
void assertion_failed(char const* expr, char const* function, char const* file,
                      long line) {
  std::stringstream s;
  s << "boost assert, expr: " << expr << " function: " << function
    << " file: " << file << " line: " << line << std::endl;
  std::cout << s.str();
  throw std::runtime_error("boost assert");
}
void assertion_failed_msg(char const* expr, char const* msg,
                          char const* function, char const* file, long line) {
  std::stringstream s;
  s << "boost assert, expr: " << expr << " msg: " << msg
    << " function: " << function << " file: " << file << " line: " << line
    << std::endl;
  std::cout << s.str();
  throw std::runtime_error(msg);
}
}  // namespace boost

#endif
