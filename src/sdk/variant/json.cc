#include "dsa_common.h"

#include "variant.h"

#include <jansson.h>

#include <iostream>

namespace dsa {

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
    return Var(json_string_value(json_obj), json_string_length(json_obj));
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
      (*map)[std::string(key, strlen(key))] = to_variant(value);
    }

    return Var(map);
  } else {
    return Var();
  }
}

Var Var::from_json(std::string data) {
  json_error_t error;
  json_t *json_obj;

  json_obj = json_loads(data.c_str(), 0, &error);

  if (!json_obj || !json_is_object(json_obj)) {
    // error handling
    return Var();
  }

  json_t *value;
  value = json_object_get(json_obj, "");

  if (!value) {
    // error handling
    return Var();
  }

  Var v = to_variant(value);

  json_decref(json_obj);

  return Var(v);
}

json_t *to_json_object(const Var &v) {
  json_t *json_obj;

  if (v.is_double()) {
    json_obj = json_real(v.get_double());
  } else if (v.is_int()) {
    json_obj = json_integer(v.get_int());
  } else if (v.is_bool()) {
    json_obj = json_boolean(v.get_bool());
  } else if (v.is_string()) {
    json_obj = json_string(v.get_string().c_str());
  } else if (v.is_binary()) {
    // TOODO
    json_obj = nullptr;
  } else if (v.is_null()) {
    json_obj = json_null();
  } else if (v.is_array()) {
    json_obj = json_array();
    VarArray &array = v.get_array();
    for (auto &it : array) {
      json_array_append_new(json_obj, to_json_object(it));
    }
  } else if (v.is_map()) {
    json_obj = json_object();

    VarMap &map = v.get_map();
    for (auto &it : map) {
      json_object_set_new_nocheck(json_obj, it.first.c_str(), to_json_object(it.second));
    }
  } else {
    json_obj = nullptr;
  }

  return json_obj;
}

std::string Var::to_json() const throw(const EncodingError &) {
  json_t *json_obj;
  char *encoded_value;

  json_obj = json_object();
  json_object_set_new_nocheck(json_obj, "", to_json_object(*this));
  encoded_value = json_dumps(json_obj, 0);

  json_decref(json_obj);

  if (encoded_value) {
    std::string return_value(encoded_value);

    free(encoded_value);

    return std::string(return_value);
  }

  throw EncodingError("Failed to encode Var to json format");
}

}  // namespace dsa
