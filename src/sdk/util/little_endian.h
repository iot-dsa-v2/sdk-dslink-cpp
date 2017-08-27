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
  uint16_t result;
  memcpy(&result, data, sizeof(uint32_t));
  return result;
}

#elif __BIG_ENDIAN__

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

#else

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

#endif
}

#endif  // DSA_SDK_LITTLE_ENDIAN_H
