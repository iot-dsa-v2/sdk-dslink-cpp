#ifndef DSA_SDK_VARIANT_H
#define DSA_SDK_VARIANT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/variant.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "util/enable_intrusive.h"
#include "util/exception.h"
#include "util/buffer.h"

struct msgpack_object;
struct json_t;

namespace dsa {

class Variant;

class VariantMap : public std::map<std::string, Variant>,
                   public EnableRef<VariantMap> {
 public:
  template <typename... Args>
  explicit VariantMap(Args &&... args)
      : std::map<std::string, Variant>(std::forward<Args>(args)...){};

  VariantMap(std::initializer_list<VariantMap::value_type> init);
};

class VariantArray : public std::vector<Variant>,
                     public EnableRef<VariantArray> {
 public:
  template <typename... Args>
  explicit VariantArray(Args &&... args)
      : std::vector<Variant>(std::forward<Args>(args)...){};

  VariantArray(std::initializer_list<Variant> init);
};

class IntrusiveString : public std::string,
                        public EnableRef<IntrusiveString> {
 public:
  template <typename... Args>
  explicit IntrusiveString(Args &&... args)
      : std::string(std::forward<Args>(args)...){};
};

typedef boost::variant<boost::blank, double, int64_t, bool, std::string,
                       ref_<IntrusiveString>,
                       ref_<VariantMap>, ref_<VariantArray>,
                       std::vector<uint8_t>, BytesRef>
    BaseVariant;

class Variant : public BaseVariant {
  enum : int {
    NUL = 0,
    DOUBLE,
    INT,
    BOOL,
    STRING,
    SHARED_STRING,
    MAP,
    ARRAY,
    BINARY,
    SHARED_BINARY
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
  Variant(Variant &&other) = default;
  Variant &operator=(const Variant &other) = default;
  Variant &operator=(Variant &&other) = default;
  ~Variant() = default;

 protected:
  explicit Variant(IntrusiveString *p);
  explicit Variant(IntrusiveBytes *p);

 public:
  bool is_double() const { return which() == DOUBLE; }
  bool is_int() const { return which() == INT; }
  bool is_bool() const { return which() == BOOL; }
  bool is_string() const {
    return which() == STRING || which() == SHARED_STRING;
  }
  bool is_map() const { return which() == MAP; }
  bool is_array() const { return which() == ARRAY; }
  bool is_binary() const {
    return which() == BINARY || which() == SHARED_BINARY;
  }
  bool is_null() const { return which() == NUL; }

  double get_double() const { return boost::get<double>(*this); }
  int64_t get_int() const { return boost::get<int64_t>(*this); }
  bool get_bool() const { return boost::get<bool>(*this); }
  const std::string &get_string() const {
    if (which() == SHARED_STRING) {
      return *boost::get<ref_<IntrusiveString>>(*this);
    }
    return boost::get<const std::string>(*this);
  }
  const std::vector<uint8_t> &get_binary() const {
    if (which() == SHARED_BINARY) {
      return *boost::get<BytesRef>(*this);
    }
    return boost::get<const std::vector<uint8_t>>(*this);
  }
  VariantMap &get_map() const {
    return *boost::get<ref_<VariantMap>>(*this);
  }
  VariantArray &get_array() const {
    return *boost::get<ref_<VariantArray>>(*this);
  }

  Variant& operator[] (const std::string &name);
  Variant& operator[] (size_t index);
 public:
  Variant deep_copy() const;

  // shallow copy on array and map
  // other types are const and can use copy constructor directly
  Variant copy() const;

  // msgpack encoding and decoding
 public:
  static Variant from_msgpack(const uint8_t *data, size_t size);
  std::vector<uint8_t> to_msgpack() const throw(const EncodingError &);

  static Variant from_json(std::string data);
  std::string to_json() const throw(const EncodingError &);

 protected:
  static Variant to_variant(const msgpack_object &obj);
  static Variant to_variant(json_t *obj);
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H
