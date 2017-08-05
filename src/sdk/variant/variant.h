#ifndef DSA_SDK_VARIANT_H
#define DSA_SDK_VARIANT_H

#include <boost/variant.hpp>
#include <map>
#include <string>
#include <vector>

#include "util/enable_shared.h"

struct msgpack_object;

namespace dsa {

class Variant;

class VariantMap : public std::map<std::string, Variant>,
                   public EnableIntrusive<VariantMap> {
 public:
  template <typename... Args>
  inline VariantMap(Args &&... args)
      : std::map<std::string, Variant>(std::forward<Args>(args)...){};
};

class VariantArray : public std::vector<Variant>,
                     public EnableIntrusive<VariantArray> {
 public:
  template <typename... Args>
  inline VariantArray(Args &&... args)
      : std::vector<Variant>(std::forward<Args>(args)...){};
};

class VariantString : public std::string, 
                      public EnableIntrusive<VariantString> {
 public:
  template <typename... Args>
  inline VariantString(Args &&... args)
      : std::string(std::forward<Args>(args)...){};
};

class VariantBinary : public std::vector<uint8_t>,
                      public EnableIntrusive<VariantBinary> {
 public:
  template <typename... Args>
  inline VariantBinary(Args &&... args)
      : std::vector<uint8_t>(std::forward<Args>(args)...){};
};

typedef boost::variant<boost::blank, double, int64_t, bool, std::string,
                       intrusive_ptr_<VariantString>,
                       intrusive_ptr_<VariantMap>, intrusive_ptr_<VariantArray>,
                       std::vector<uint8_t>, intrusive_ptr_<VariantBinary>>
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

 protected:
  explicit Variant(int64_t v);
  explicit Variant(double v);
  explicit Variant(bool v);

  explicit Variant(VariantMap *p);
  explicit Variant(VariantArray *p);

  explicit Variant(const char *v, size_t size);
  explicit Variant(const std::string &v);
  explicit Variant(const std::string *p);

  explicit Variant(const uint8_t *data, size_t size);
  explicit Variant(const std::vector<uint8_t> &v);
  explicit Variant(const std::vector<uint8_t> *p);

 public:
  static Variant *new_map();
  static Variant *new_array();

  template <typename T>
  static Variant create(const T &v) {
    return Variant(v);
  };
  static Variant create(const int32_t &v) {
    return Variant(static_cast<int64_t>(v));
  }
  static Variant create(const uint64_t &v) {
    return Variant(static_cast<int64_t>(v));
  }
  static Variant create(const std::string &v) {
    if (v.length() < 64) {
      return Variant(v);
    } else {
      return Variant(new std::string(v));
    }
  }
  static Variant create(const char *v) {
    size_t size = strlen(v);
    return create(v, size);
  }
  static Variant create(const char *v, size_t size) {
    if (size < 64) {
      return Variant(v, size);
    } else {
      return Variant(new std::string(v, size));
    }
  }
  static Variant create(const std::vector<uint8_t> &v) {
    if (v.size() < 64) {
      return Variant(v);
    } else {
      return Variant(new std::vector<uint8_t>(v));
    }
  }
  static Variant create(const uint8_t *data, size_t size) {
    if (size < 64) {
      return Variant(data, size);
    } else {
      return Variant(new std::vector<uint8_t>(data, data + size));
    }
  }

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
      return *boost::get<intrusive_ptr_<VariantString>>(*this);
    }
    return boost::get<const std::string>(*this);
  }
  const std::vector<uint8_t> &get_binary() const {
    if (which() == SharedBinary) {
      return *boost::get<intrusive_ptr_<VariantBinary>>(*this);
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
  Variant *deep_copy() const;

  // shallow copy on array and map
  // other types are const and can use copy constructor directly
  Variant *copy() const;

  // msgpack encoding and decoding
 public:
  static Variant *from_msgpack(const uint8_t *data, size_t size);
  std::vector<uint8_t> *to_msgpack();

 protected:
  static Variant to_variant(const msgpack_object &obj);
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H