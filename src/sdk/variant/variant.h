#ifndef DSA_SDK_VARIANT_H
#define DSA_SDK_VARIANT_H

#include <boost/variant.hpp>
#include <map>
#include <string>
#include <vector>

#include "../util/buffer.h"

namespace dsa {
class Variant;
typedef boost::variant<double, int64_t, bool, std::shared_ptr<std::string>,
                       std::shared_ptr<std::map<std::string, Variant>>,
                       std::shared_ptr<std::vector<Variant>>, BufferPtr,
                       boost::blank>
    BaseVariant;

class Variant : public BaseVariant {
  enum : int { Double = 0, Int, Bool, String, Map, Array, Binary, Null };

 public:
  Variant(int64_t v);
  Variant(double v);
  Variant(bool v);
  Variant(const std::string& v);
  Variant(const char* v);

 protected:
  Variant(const std::shared_ptr<std::map<std::string, Variant>>& v);
  Variant(const std::shared_ptr<std::vector<Variant>>& v);
  Variant(const BufferPtr& v);
  Variant(const boost::blank& v);

 public:
  static Variant new_map();
  static Variant new_array();
  static Variant null();

 public:
  bool is_double() { return which() == Double; }
  bool is_int() { return which() == Int; }
  bool is_bool() { return which() == Bool; }
  bool is_string() { return which() == String; }
  bool is_map() { return which() == Map; }
  bool is_array() { return which() == Array; }
  bool is_binary() { return which() == Binary; }
  bool is_null() { return which() == Null; }
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H
