#ifndef DSA_SDK_VARIANT_H
#define DSA_SDK_VARIANT_H

#include <boost/variant.hpp>
#include <map>
#include <string>
#include <vector>

namespace dsa {
class Variant;
typedef boost::variant<boost::blank, double, int64_t, bool,
                       std::shared_ptr<std::string>,
                       std::shared_ptr<std::map<std::string, Variant>>,
                       std::shared_ptr<std::vector<Variant>>,
                       std::shared_ptr<const std::vector<uint8_t>>>
    BaseVariant;

class Variant : public BaseVariant {
  enum : int { Null = 0, Double, Int, Bool, String, Map, Array, Binary };

 public:
  explicit Variant(int64_t v);
  explicit Variant(int32_t v);
  explicit Variant(int16_t v);
  explicit Variant(double v);
  explicit Variant(bool v);
  explicit Variant(const std::string &v);
  explicit Variant(const char *v);
  explicit Variant(const char *v, size_t size);
  explicit Variant(const std::vector<uint8_t> &v);
  explicit Variant(const uint8_t *data, size_t size);

  Variant();

  explicit Variant(const std::shared_ptr<std::map<std::string, Variant>> &v);
  explicit Variant(const std::shared_ptr<std::vector<Variant>> &v);

 public:
  static Variant new_map();
  static Variant new_array();

 public:
  static Variant from_msgpack(const uint8_t *data, size_t size);
  std::vector<const std::vector<uint8_t> *> to_msgpack();

  bool is_double() const { return which() == Double; }
  bool is_int() const { return which() == Int; }
  bool is_bool() const { return which() == Bool; }
  bool is_string() const { return which() == String; }
  bool is_map() const { return which() == Map; }
  bool is_array() const { return which() == Array; }
  bool is_binary() const { return which() == Binary; }
  bool is_null() const { return which() == Null; }

  double get_double() const { return boost::get<double>(*this); }
  int64_t get_int() const { return boost::get<int64_t>(*this); }
  bool get_bool() const { return boost::get<bool>(*this); }
  const std::string &get_string() const {
    return *boost::get<std::shared_ptr<std::string>>(*this);
  }
  std::map<std::string, Variant> &get_map() const {
    return *boost::get<std::shared_ptr<std::map<std::string, Variant>>>(*this);
  }
  std::vector<Variant> &get_array() const {
    return *boost::get<std::shared_ptr<std::vector<Variant>>>(*this);
  }
  const std::vector<uint8_t> &get_binary() const {
    return *boost::get<std::shared_ptr<const std::vector<uint8_t>>>(*this);
  }

 public:
  // shallow copy on array and map
  // other types are const and can use copy constructor directly
  Variant copy() const;
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H
