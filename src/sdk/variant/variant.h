#ifndef DSA_SDK_VARIANT_H
#define DSA_SDK_VARIANT_H

#include <boost/variant.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "util/enable_intrusive.h"
#include "util/exception.h"
#include "util/buffer.h"

struct msgpack_object;

namespace dsa {

class Variant;

class VariantMap : public std::map<std::string, Variant>,
                   public EnableIntrusive<VariantMap> {
 public:
  template <typename... Args>
  inline VariantMap(Args &&... args)
      : std::map<std::string, Variant>(std::forward<Args>(args)...){};

  VariantMap(std::initializer_list<VariantMap::value_type> init);
};

class VariantArray : public std::vector<Variant>,
                     public EnableIntrusive<VariantArray> {
 public:
  template <typename... Args>
  inline VariantArray(Args &&... args)
      : std::vector<Variant>(std::forward<Args>(args)...){};

  VariantArray(std::initializer_list<Variant> init);
};

class IntrusiveString : public std::string,
                        public EnableIntrusive<IntrusiveString> {
 public:
  template <typename... Args>
  inline IntrusiveString(Args &&... args)
      : std::string(std::forward<Args>(args)...){};
};

typedef boost::variant<boost::blank, double, int64_t, bool, std::string,
                       intrusive_ptr_<IntrusiveString>,
                       intrusive_ptr_<VariantMap>, intrusive_ptr_<VariantArray>,
                       std::vector<uint8_t>, intrusive_ptr_<ByteBuffer>>
    BaseVariant;

class Variant : public BaseVariant {
  enum : int {
    Null = 0,
    Double,
    Int,
    Bool,
    String,
    SharedString,
    Map,
    Array,
    Binary,
    SharedBinary
  };

 public:
  Variant();

  explicit Variant(int64_t v);
  explicit Variant(int32_t v);
  explicit Variant(uint64_t v);

  explicit Variant(double v);
  explicit Variant(bool v);

  explicit Variant(VariantMap *p);
  explicit Variant(VariantArray *p);

  explicit Variant(const char *v);
  explicit Variant(const char *v, size_t size);
  explicit Variant(const std::string &v);
  explicit Variant(const std::string &&v);

  explicit Variant(const uint8_t *data, size_t size);
  explicit Variant(const std::vector<uint8_t> &v);
  explicit Variant(const std::vector<uint8_t> &&v);

 public:
  Variant(std::initializer_list<VariantMap::value_type> init);
  Variant(std::initializer_list<Variant> init);
  static Variant new_map();
  static Variant new_array();

 template <class T>
  inline Variant & operator= (T && other){
    BaseVariant::operator=(std::forward<T>(other));
    return *this;
  }

  Variant(const Variant &other) = default;
  Variant(Variant &&other) noexcept = default;
  Variant &operator=(const Variant &other) = default;
  Variant &operator=(Variant &&other) noexcept = default;
  ~Variant() = default;

 protected:
  explicit Variant(IntrusiveString *p);
  explicit Variant(ByteBuffer *p);

 public:
  bool is_double() const { return which() == Double; }
  bool is_int() const { return which() == Int; }
  bool is_bool() const { return which() == Bool; }
  bool is_string() const {
    return which() == String || which() == SharedString;
  }
  bool is_map() const { return which() == Map; }
  bool is_array() const { return which() == Array; }
  bool is_binary() const {
    return which() == Binary || which() == SharedBinary;
  }
  bool is_null() const { return which() == Null; }

  double get_double() const { return boost::get<double>(*this); }
  int64_t get_int() const { return boost::get<int64_t>(*this); }
  bool get_bool() const { return boost::get<bool>(*this); }
  const std::string &get_string() const {
    if (which() == SharedString) {
      return *boost::get<intrusive_ptr_<IntrusiveString>>(*this);
    }
    return boost::get<const std::string>(*this);
  }
  const std::vector<uint8_t> &get_binary() const {
    if (which() == SharedBinary) {
      return *boost::get<intrusive_ptr_<ByteBuffer>>(*this);
    }
    return boost::get<const std::vector<uint8_t>>(*this);
  }
  VariantMap &get_map() const {
    return *boost::get<intrusive_ptr_<VariantMap>>(*this);
  }
  VariantArray &get_array() const {
    return *boost::get<intrusive_ptr_<VariantArray>>(*this);
  }

 public:
  Variant deep_copy() const;

  // shallow copy on array and map
  // other types are const and can use copy constructor directly
  Variant copy() const;

  // msgpack encoding and decoding
 public:
  static Variant from_msgpack(const uint8_t *data, size_t size);
  std::vector<uint8_t> to_msgpack() throw(const EncodingError &);

 protected:
  static Variant to_variant(const msgpack_object &obj);
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H
