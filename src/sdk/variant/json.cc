#include "dsa_common.h"

#include "variant.h"

#include <cmath>
#include "crypto/misc.h"
#include "jansson_9e7847e.h"

namespace dsa {

enum JsonSpecialType {
  JSON_NORMAL = 0,
  JSON_NAN,
  JSON_INFINITY,
  JSON_N_INFINITY,
  JSON_BINARY
};
static JsonSpecialType is_json_special(const char *str) {
  if (*str != '\u001B') return JSON_NORMAL;
  if (strncmp(str + 1, "bytes:", 6) == 0) {
    return JSON_BINARY;
  }
  if (strcmp(str + 1, "NaN") == 0) {
    return JsonSpecialType::JSON_NAN;
  }
  if (strcmp(str + 1, "Infinity") == 0) {
    return JsonSpecialType::JSON_INFINITY;
  }
  if (strcmp(str + 1, "-Infinity") == 0) {
    return JsonSpecialType::JSON_N_INFINITY;
  }
  return JsonSpecialType::JSON_NORMAL;
}

json_t *json_binary(const uint8_t *bin_value, size_t bin_len) {
  string_ base64 = "\u001Bbytes:" + base64_encode(bin_value, bin_len);

  return json_string(base64.c_str());
}

Var Var::to_variant(json_t *json_obj) {
  if (json_is_integer(json_obj)) {
    return Var(static_cast<int64_t>(json_integer_value(json_obj)));
  } else if (json_is_real(json_obj)) {
    return Var(static_cast<double>(json_real_value(json_obj)));
  } else if (json_is_true(json_obj)) {
    return Var(true);
  } else if (json_is_false(json_obj)) {
    return Var(false);
  } else if (json_is_string(json_obj)) {
    const char *str = json_string_value(json_obj);
    JsonSpecialType t = is_json_special(str);
    switch (t) {
      case JSON_NORMAL:
        return Var(json_string_value(json_obj), json_string_length(json_obj));
      case JSON_BINARY:
        return Var(base64_decode(str + 7));
      case JSON_NAN:
        return Var(NAN);
      case JSON_INFINITY:
        return Var(INFINITY);
      case JSON_N_INFINITY:
        return Var(-INFINITY);
      default:
        // impossible to reach here
        return Var();
    }
  } else if (json_is_array(json_obj)) {
    auto array = new VarArray();
    array->reserve(json_array_size(json_obj));

    size_t index;
    json_t *value;
    json_array_foreach(json_obj, index, value) {
      array->push_back(to_variant(value));
    }

    return Var(array);
  } else if (json_is_object(json_obj)) {
    auto map = new VarMap();

    const char *key;
    json_t *value;
    json_object_foreach(json_obj, key, value) {
      (*map)[string_(key, strlen(key))] = to_variant(value);
    }

    return Var(map);
  } else {
    return Var();
  }
}

Var Var::from_json(const char *data, size_t len) {
  json_error_t error;
  json_t *json_obj;

  json_obj = json_loadb(data, len, 0, &error);

  if (!json_obj) {
    // error handling
    return Var();
  }

  Var v = to_variant(json_obj);

  json_decref(json_obj);

  return Var(v);
}
Var Var::from_json(const string_ &str) {
  return from_json(str.data(), str.size());
}

json_t *to_json_object(const Var &v) {
  if (v.is_double()) {
    double d = v.get_double();
    if (d != d) {
      return json_string("\u001BNaN");
    } else if (d == INFINITY) {
      return json_string("\u001BInfinity");
    } else if (d == -INFINITY) {
      return json_string("\u001BInfinity");
    }
    return json_real(v.get_double());
  } else if (v.is_int()) {
    return json_integer(v.get_int());
  } else if (v.is_bool()) {
    return json_boolean(v.get_bool());
  } else if (v.is_string()) {
    return json_string(v.get_string().c_str());
  } else if (v.is_binary()) {
    auto bin = v.get_binary();
    return json_binary(bin.data(), bin.size());
  } else if (v.is_null()) {
    return json_null();
  } else if (v.is_array()) {
    json_t *json_obj = json_array();
    VarArray &array = v.get_array();
    for (auto &it : array) {
      json_array_append_new(json_obj, to_json_object(it));
    }
    return json_obj;
  } else if (v.is_map()) {
    json_t *json_obj = json_object();
    VarMap &map = v.get_map();
    for (auto &it : map) {
      json_object_set_new_nocheck(json_obj, it.first.c_str(),
                                  to_json_object(it.second));
    }
    return json_obj;
  } else {
    return json_null();
  }
}

string_ Var::to_json(size_t indent) const throw(const EncodingError &) {
  json_t *json_obj = to_json_object(*this);
  char *encoded_value;

  if (indent == 0) {
    encoded_value = json_dumps(json_obj, JSON_COMPACT | JSON_ENCODE_ANY);
  } else {
    if (indent > JSON_MAX_INDENT) indent = JSON_MAX_INDENT;
    encoded_value = json_dumps(
        json_obj, JSON_SORT_KEYS | JSON_ENCODE_ANY | JSON_INDENT(indent));
  }

  json_decref(json_obj);

  if (encoded_value) {
    string_ return_value(encoded_value);

    free(encoded_value);

    return string_(return_value);
  }

  throw EncodingError("Failed to encode Var to json format");
}

}  // namespace dsa
