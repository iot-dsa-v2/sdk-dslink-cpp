#include "dsa_common.h"

#include "buffer.h"
#include "message/base_message.h"

namespace dsa {

Buffer::Buffer()
    : _data(new uint8_t[default_capacity]), _size(0), _capacity(default_capacity) {}

Buffer::~Buffer() {
  delete[] _data;
}

Buffer::Buffer(size_t capacity)
    : _size(0), _capacity(capacity) {
  if (capacity < 1)
    _capacity = default_capacity;
  _data = new uint8_t[_capacity];
}

Buffer::Buffer(const Buffer &other)
    : _size(other._size), _capacity(other._capacity) {
  _data = new uint8_t[_capacity];
  std::memcpy(_data, other.data(), other.size());
}

Buffer::Buffer(uint8_t *data, size_t size, size_t capacity)
    : _data(data), _size(size), _capacity(capacity) {}

Buffer::Buffer(std::string data) : _size(data.size()), _capacity(data.size()) {
  _data = new uint8_t[_capacity];
  std::memcpy(_data, data.c_str(), data.size());
}

Buffer &Buffer::operator=(const Buffer &other) throw(const std::runtime_error &) {
  if (_refs > 2) {
    throw std::runtime_error("Can't not resize a buffer that's already shared");
  }
  delete[] _data;
  _data = new uint8_t[other.capacity()];
  _size = other.size();
  _capacity = other.capacity();
  std::memcpy(_data, other.data(), other.size());
  return *this;
}

size_t Buffer::capacity() const { return _capacity; }

size_t Buffer::size() const { return _size; }

bool Buffer::resize(size_t capacity) throw(const std::runtime_error &) {
  if (_refs > 2) {
    throw std::runtime_error("Buffer resize, can't resize a buffer that's already shared");
  }
  if (capacity <= _capacity)
    return false;
  auto new_data = new uint8_t[capacity];
  std::memcpy(new_data, _data, _size);
  delete[] _data;
  _data = new_data;
  _capacity = capacity;
  return true;
}

void Buffer::append(uint8_t data) { _data[_size++] = data; }

void Buffer::append(const Message &message) {
  message.write(_data + _size);
  _size += message.size();
}

void Buffer::safe_append(uint8_t data) {
  if (_size >= _capacity)
    resize(_capacity * 2);
  _data[_size++] = data;
}

const uint8_t *Buffer::data() const { return _data; }

uint8_t *Buffer::data() { return _data; }

void Buffer::assign(const uint8_t *data, size_t size) throw(const std::runtime_error &) {
  if (_capacity < size) {
    throw std::runtime_error("Buffer assign, not enough capacity");
  }
  _size = size;
  std::memcpy(_data, data, _size);
}

uint8_t &Buffer::operator[](size_t index) throw(const std::runtime_error &) {
  if (index >= _size) {
    if (index >= _capacity) {
      throw std::runtime_error("Buffer access, index out of bounds");
    }
    _size = index + 1;
  }
  return _data[index];
}

const uint8_t &Buffer::operator[](size_t index) const throw(const std::runtime_error &) {
  if (index >= _size) {
    throw std::runtime_error("Buffer access, index out of bounds");
  }
  return _data[index];
}

Buffer::SharedBuffer Buffer::get_shared_buffer(size_t offset, size_t message_size) {
  return Buffer::SharedBuffer(intrusive_this(), &_data[offset], message_size);
}

} // namespace dsa
