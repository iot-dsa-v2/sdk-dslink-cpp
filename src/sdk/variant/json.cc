#include "dsa_common.h"

#include "variant.h"

#include <jansson.h>

#include <iostream>

namespace dsa {

Variant Variant::from_json(std::string data) {
  ;
}

json_t *to_json_object(const Variant &v) {
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
    json_obj = nullptr;
  } else if (v.is_map()) {
    json_obj = nullptr;
  } else {
    json_obj = nullptr;
  }

  return json_obj;
}

std::string Variant::to_json() const throw(const EncodingError &) {
  json_t *json_obj;
  // json_obj = to_json_object(*this);

  char *encoded_value;
  // encoded_value = json_dumps(json_obj, 0);

  json_obj = json_object();
  json_object_set_new_nocheck(json_obj, "", to_json_object(*this));
  encoded_value = json_dumps(json_obj, 0);

  json_decref(json_obj);

  if (encoded_value) {
    std::string return_value(encoded_value);

    free(encoded_value);

    return std::string(return_value);
  }

  throw EncodingError("Failed to encode Variant to json format");
}

}  // namespace dsa
