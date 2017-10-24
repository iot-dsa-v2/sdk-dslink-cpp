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

#include "util/buffer.h"
#include "util/enable_intrusive.h"
#include "util/exception.h"

struct msgpack_object;
struct json_t;

namespace dsa {

class Var;

class VarMap : public std::map<std::string, Var>, public EnableRef<VarMap> {
 public:
  template <typename... Args>
  explicit VarMap(Args &&... args)
      : std::map<std::string, Var>(std::forward<Args>(args)...){};

  VarMap(std::initializer_list<VarMap::value_type> init);
};

class VarArray : public std::vector<Var>, public EnableRef<VarArray> {
 public:
  template <typename... Args>
  explicit VarArray(Args &&... args)
      : std::vector<Var>(std::forward<Args>(args)...){};

  VarArray(std::initializer_list<Var> init);
};

class IntrusiveString : public std::string, public EnableRef<IntrusiveString> {
 public:
  template <typename... Args>
  explicit IntrusiveString(Args &&... args)
      : std::string(std::forward<Args>(args)...){};
};

typedef boost::variant<boost::blank, double, int64_t, bool, std::string,
                       ref_<IntrusiveString>, ref_<VarMap>, ref_<VarArray>,
                       std::vector<uint8_t>, BytesRef>
    BaseVariant;

class Var : public BaseVariant {
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
  Var();

  explicit Var(int64_t v);
  explicit Var(int32_t v);
  explicit Var(uint64_t v);

  explicit Var(double v);
  explicit Var(bool v);

  explicit Var(VarMap *p);
  explicit Var(VarArray *p);

  explicit Var(const char *v);
  explicit Var(const char *v, size_t size);
  explicit Var(const std::string &v);
  explicit Var(const std::string &&v);

  explicit Var(const uint8_t *data, size_t size);
  explicit Var(const std::vector<uint8_t> &v);
  explicit Var(const std::vector<uint8_t> &&v);

 public:
  Var(std::initializer_list<VarMap::value_type> init);
  Var(std::initializer_list<Var> init);
  static Var new_map();
  static Var new_array();

  template <class T>
  inline Var &operator=(T &&other) {
    BaseVariant::operator=(std::forward<T>(other));
    return *this;
  }

  Var(const Var &other) = default;
  Var(Var &&other) = default;
  Var &operator=(const Var &other) = default;
  Var &operator=(Var &&other) = default;
  ~Var() = default;

 protected:
  explicit Var(IntrusiveString *p);
  explicit Var(IntrusiveBytes *p);

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
  VarMap &get_map() const { return *boost::get<ref_<VarMap>>(*this); }
  VarArray &get_array() const { return *boost::get<ref_<VarArray>>(*this); }

  double to_double(double defaultout = 0.0 / 0.0) const;
  int64_t to_bool(bool defaultout = false) const;
  const std::string to_string(const std::string &defaultout = "") const;

  Var &operator[](const std::string &name);
  Var &operator[](size_t index);

 public:
  Var deep_copy() const;

  // shallow copy on array and map
  // other types are const and can use copy constructor directly
  Var copy() const;

  // msgpack encoding and decoding
 public:
  static Var from_msgpack(const uint8_t *data, size_t size);
  std::vector<uint8_t> to_msgpack() const throw(const EncodingError &);

  static Var from_json(std::string data);
  std::string to_json() const throw(const EncodingError &);

 protected:
  static Var to_variant(const msgpack_object &obj);
  static Var to_variant(json_t *obj);
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H
