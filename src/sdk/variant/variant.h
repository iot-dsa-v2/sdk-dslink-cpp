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
#include "util/enable_ref.h"
#include "util/enums.h"
#include "util/exception.h"

struct msgpack_object;
struct json_t;

namespace dsa {

class Var;

class VarMap : public std::map<string_, Var>, public EnableRef<VarMap> {
 public:
  template <typename... Args>
  explicit VarMap(Args &&... args)
      : std::map<string_, Var>(std::forward<Args>(args)...){};

  VarMap(std::initializer_list<VarMap::value_type> init);
};

class VarArray : public std::vector<Var>, public EnableRef<VarArray> {
 public:
  template <typename... Args>
  explicit VarArray(Args &&... args)
      : std::vector<Var>(std::forward<Args>(args)...){};

  VarArray(std::initializer_list<Var> init);
};

class RefCountString : public string_, public EnableRef<RefCountString> {
 public:
  template <typename... Args>
  explicit RefCountString(Args &&... args)
      : string_(std::forward<Args>(args)...){};
};

// dsa::ref_ being used instead of just ref_ is
// only a workaround for clion code analyzer bug
using BaseVariant =
    boost::variant<boost::blank, double, int64_t, bool, string_,
                   dsa::ref_<const RefCountString>, dsa::ref_<VarMap>,
                   dsa::ref_<VarArray>, std::vector<uint8_t>, BytesRef,
                   StatusDetail>;

class Var : public BaseVariant {
 public:
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
    SHARED_BINARY,
    STATUS
  };

  static const size_t MAX_PAGE_BODY_SIZE = 0xC000;

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
  explicit Var(const string_ &v);
  explicit Var(const string_ &&v);

  explicit Var(const uint8_t *data, size_t size);
  explicit Var(const std::vector<uint8_t> &v);
  explicit Var(const std::vector<uint8_t> &&v);

  explicit Var(MessageStatus status, const string_ &detail = "");

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
  explicit Var(RefCountString *p);
  explicit Var(RefCountBytes *p);

 public:
  // return the type infomation that hides shared_string and shared_binary
  int get_type();

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
  bool is_status() const { return which() == STATUS; }

  const StatusDetail &get_status() { return boost::get<StatusDetail>(*this); }
  double get_double() const { return boost::get<double>(*this); }
  int64_t get_int() const { return boost::get<int64_t>(*this); }
  bool get_bool() const { return boost::get<bool>(*this); }
  const string_ &get_string() const {
    if (which() == SHARED_STRING) {
      return *boost::get<ref_<const RefCountString>>(*this);
    }
    return boost::get<const string_>(*this);
  }
  const std::vector<uint8_t> &get_binary() const {
    if (which() == SHARED_BINARY) {
      return *boost::get<BytesRef>(*this);
    }
    return boost::get<const std::vector<uint8_t>>(*this);
  }
  const BytesRef get_bytesref() const {
    if (which() == SHARED_BINARY) {
      return boost::get<BytesRef>(*this);
    }
    return make_ref_<const RefCountBytes>(
        boost::get<const std::vector<uint8_t>>(*this));
  }
  VarMap &get_map() const { return *boost::get<ref_<VarMap>>(*this); }
  VarArray &get_array() const { return *boost::get<ref_<VarArray>>(*this); }

  double to_double(double defaultout = 0.0 / 0.0) const;
  int64_t to_bool(bool defaultout = false) const;
  const string_ to_string(const string_ &defaultout = "") const;

  // safe operation, return null value Var when not found
  Var &operator[](const string_ &name);
  // safe operation, return null value Var when out of range
  Var &operator[](size_t index);

 public:
  Var deep_copy() const;

  // shallow copy on array and map
  // other types are const and can use copy constructor directly
  Var copy() const;

  // shallow compare
  bool operator==(const Var &lhs) const;
  bool operator!=(const Var &lhs) const;

  // msgpack encoding and decoding
 public:
  static Var from_msgpack(const uint8_t *data, size_t size);
  std::vector<uint8_t> to_msgpack() const throw(const EncodingError &);

  // extra_first_page and first_size are for MessageValue type
  // where the first page is not a whole BytesRef
  static Var from_msgpack_pages(std::vector<BytesRef> &pages,
                                const uint8_t *extra_first_page = nullptr,
                                size_t first_size = 0);

  std::vector<BytesRef> to_msgpack_pages(
      size_t first_page_size = MAX_PAGE_BODY_SIZE) const
      throw(const EncodingError &);
  static std::vector<BytesRef> split_pages(
      const std::vector<uint8_t> &data,
      size_t first_page_size = MAX_PAGE_BODY_SIZE);

  static Var from_json(const char *data, size_t len);
  static Var from_json(const string_ &str);
  string_ to_json(size_t indent = 0) const throw(const EncodingError &);

 protected:
  static Var to_variant(const msgpack_object &obj);
  static Var to_variant(json_t *obj);
};

// VarByte is used to decode and encode Var on demand
class VarBytes : public EnableRef<VarBytes> {
  mutable BytesRef _bytes;
  mutable Var _v;

 public:
  VarBytes() : _bytes(new RefCountBytes()) {}
  VarBytes(Var &&v) : _v(std::move(v)) {}
  VarBytes(const Var &v) : _v(v) {}
  VarBytes(BytesRef bytes) : _bytes(std::move(bytes)) {}
  VarBytes(std::vector<uint8_t> &&bytes)
      : _bytes(new RefCountBytes(std::move(bytes))) {}

  BytesRef &get_bytes() const {
    if (_bytes == nullptr) {
      _bytes.reset(new RefCountBytes(_v.to_msgpack()));
    }
    return _bytes;
  }
  Var &get_value() const {
    if (_v.is_null() && _bytes != nullptr && _bytes->size() != 0) {
      _v = Var::from_msgpack(&(*_bytes)[0], _bytes->size());
    }
    return _v;
  }
  // blank
  inline bool is_blank() const {
    return _bytes != nullptr && _bytes->size() == 0;
  }
  inline size_t size() const { return get_bytes()->size(); }
  bool operator==(const Var &other) const { return other == get_value(); };
  bool operator==(const VarBytes &other) const {
    return other.get_value() == get_value();
  };
};

typedef ref_<VarBytes> VarBytesRef;

typedef std::function<bool(const Var &)> VarValidator;

class VarValidatorInt {
  int64_t _min;
  int64_t _max;

 public:
  VarValidatorInt(int64_t min, int64_t max) : _min(min), _max(max){};
  bool operator()(const Var &var) {
    return var.is_int() && var.get_int() >= _min && var.get_int() <= _max;
  }
};

}  // namespace dsa

#endif  // DSA_SDK_VARIANT_H
