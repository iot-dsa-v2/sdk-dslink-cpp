#ifndef DSA_SDK_UTIL_ENABLE_CLOSABLE_H_
#define DSA_SDK_UTIL_ENABLE_CLOSABLE_H_

namespace dsa {

class Closable {
 private:
  bool _closed{false};

 public:
  bool is_closed() const { return _closed; }
  virtual void close() { _closed = true; }
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_ENABLE_CLOSABLE_H_
