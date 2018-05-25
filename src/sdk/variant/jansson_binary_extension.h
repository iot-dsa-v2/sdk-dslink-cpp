
#ifndef PROJECT_JANSSON_BINARY_EXTENSION_H
#define PROJECT_JANSSON_BINARY_EXTENSION_H

#include "crypto/misc.h"
#include "jansson_9e7847e.h"

using namespace dsa;

static JSON_INLINE bool is_binary_json(const json_t *json_obj) {
  auto len = json_string_length(json_obj);
  // it should begin with \u001Bbytes: + 1 byte data
  if (len < 8) return false;

  auto binary_str = json_string_value(json_obj);
  if (binary_str[0] != '\u001B') return false;
  if (binary_str[1] != 'b') return false;
  if (binary_str[2] != 'y') return false;
  if (binary_str[3] != 't') return false;
  if (binary_str[4] != 'e') return false;
  if (binary_str[5] != 's') return false;
  if (binary_str[6] != ':') return false;

  return true;
}

#define json_is_binary(json) is_binary_json(json)

std::vector<uint8_t> json_binary_value(const json_t *binary_json) {
  const char *raw_data = json_string_value(binary_json);
  size_t raw_len = json_string_length(binary_json);

  string_ base64 = string_(raw_data, raw_len);
  // remove prefix
  base64.erase(0, 7);

  return base64_decode(base64);
}

json_t *json_binary(const uint8_t *bin_value, size_t bin_len) {
  string_ base64 = "\u001Bbytes:" + base64_encode(bin_value, bin_len);

  return json_string(base64.c_str());
}

#endif  // PROJECT_JANSSON_BINARY_EXTENSION_H
