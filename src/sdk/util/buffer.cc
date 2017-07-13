#include "buffer.h"

#include <cstring>
#include <iostream>

#include <boost/asio.hpp>

namespace dsa {

template <typename T>
Buffer<T>::Buffer()
    : _data(new T[default_capacity]),
      _size(0),
      _capacity(default_capacity) {}

template <typename T>
Buffer<T>::Buffer(size_t capacity)
    : _data(new T[capacity]), _size(0), _capacity(capacity) {
  if (capacity < 1)
    throw new std::runtime_error("Invalid capacity for buffer constructor");
}

template <typename T>
Buffer<T>::Buffer(const Buffer& other)
    : _size(other._size), _capacity(other._capacity) {
  delete[] _data;
  _data = new T[_capacity];
  std::memcpy(_data, other.data(), other.size());
}

template <typename T>
Buffer<T>::Buffer(T* data, size_t size, size_t capacity)
    : _data(data), _size(size), _capacity(capacity) {}

template <typename T>
Buffer<T>& Buffer<T>::operator=(const Buffer<T>& other) {
  delete[] _data;
  _data = new T[other.capacity()];
  _size = other.size();
  _capacity = other.capacity();
  std::memcpy(_data, other.data(), other.size());
  return *this;
}

template <typename T>
size_t Buffer<T>::capacity() {
  return _capacity;
}

template <typename T>
bool Buffer<T>::resize(size_t capacity) {
  if (capacity <= _capacity) return false;
  T* new_data = new T[capacity];
  std::memcpy(new_data, _data, _size);
  delete[] _data;
  _data = new_data;
  _capacity = capacity;
  return true;
}

template <typename T>
size_t Buffer<T>::size() {
  return _size;
}

template <typename T>
void Buffer<T>::append(T data) {
  _data[_size++] = data;
}

template <typename T>
void Buffer<T>::safe_append(T data) {
  if (_size + 1 >= _capacity)
    resize(_capacity * 2);
  _data[_size++] = data;
}

template <typename T>
T* Buffer<T>::data() {
  return _data;
}

template <typename T>
T& Buffer<T>::operator[](int index) {
  if (index >= _size) {
    if (index >= _capacity) {
      throw new std::runtime_error("Buffer access, index out of bounds");
    }
    _size = index + 1;
  }
  return _data[index];
}

}  // namespace dsa
