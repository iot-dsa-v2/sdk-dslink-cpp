#ifndef DSA_SDK_LITTLE_ENDIAN_H
#define DSA_SDK_LITTLE_ENDIAN_H

namespace dsa {

#ifdef __arm__

static size_t write_16_t(uint8_t* data, uint16_t n) {
  memcpy(data, &n, sizeof(uint16_t));
  return sizeof(uint16_t);
}

static size_t write_32_t(uint8_t* data, uint32_t n) {
  memcpy(data, &n, sizeof(uint32_t));
  return sizeof(uint32_t);
}

static uint16_t read_16_t(const uint8_t* data) {
  uint16_t result;
  memcpy(&result, data, sizeof(uint16_t));
  return result;
}

static uint32_t read_32_t(const uint8_t* data) {
  uint32_t result;
  memcpy(&result, data, sizeof(uint32_t));
  return result;
}

#else // __arm__

#ifdef __BIG_ENDIAN__

static size_t write_16_t(uint8_t* data, uint16_t n) {
  uint8_t* raw = reinterpret_cast<uint8_t*> & n;
  data[0] = raw[1];
  data[1] = raw[0];
  return sizeof(uint16_t);
}

static size_t write_32_t(uint8_t* data, uint32_t n) {
  uint32_t swap32 = __builtin_bswap32(n);
  memcpy(data, &swap32, sizeof(uint32_t));
  return sizeof(uint32_t);
}

static uint16_t read_16_t(const uint8_t* data) {
  uint16_t result;
  uint8_t* raw = reinterpret_cast<const uint8_t*> & result;
  raw[0] = data[1];
  raw[1] = data[0];
  return result;
}

static uint32_t read_32_t(const uint8_t* data) {
  return __builtin_bswap32(*reinterpret_cast<const uint32_t*>(data));
}

#else // __BIG_ENDIAN__

static size_t write_16_t(uint8_t* data, uint16_t n) {
  memcpy(data, &n, sizeof(uint16_t));
  return sizeof(uint16_t);
}

static size_t write_32_t(uint8_t* data, uint32_t n) {
  memcpy(data, &n, sizeof(uint32_t));
  return sizeof(uint32_t);
}

static uint16_t read_16_t(const uint8_t* data) {
  return *reinterpret_cast<const uint16_t*>(data);
}

static uint32_t read_32_t(const uint8_t* data) {
  return *reinterpret_cast<const uint32_t*>(data);
}

#endif // __BIG_ENDIAN__
#endif // __arm__

// write str into data, return total bytes used in data
static size_t write_str_with_len(uint8_t* data, const std::string& str) {
  write_16_t(data, str.length());
  str.copy(reinterpret_cast<char*>(data + sizeof(uint16_t)), str.length());
  return str.length() + sizeof(uint16_t);
}

// read data into str, return total bytes used in data
static size_t read_str_with_len(const uint8_t* data, std::string& str) {
  size_t len = read_16_t(data);
  str.assign(reinterpret_cast<const char*>(data + sizeof(uint16_t)), len);
  return len + sizeof(uint16_t);
}
}

#endif  // DSA_SDK_LITTLE_ENDIAN_H
