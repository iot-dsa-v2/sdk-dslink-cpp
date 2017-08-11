#ifndef DSA_SDK_UTIL_ENABLE_CLOSABLE_H_
#define DSA_SDK_UTIL_ENABLE_CLOSABLE_H_

namespace dsa {

class Closable {
 public:
  virtual void close() = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_CLOSABLE_H_
