#ifndef DSA_SDK_UTIL_BUFFER_H_
#define DSA_SDK_UTIL_BUFFER_H_

#include <vector>
#include <memory>

#include <boost/asio.hpp>

namespace dsa {
template<typename T>
class Buffer {
 private:
  enum { default_capacity = 256 };
  T * _data;
  size_t _size;
  size_t _capacity;
 public:
  // default constructor
  Buffer();

  // set capacity constructor
  Buffer(size_t capacity);

  // shallow copy constructor
  Buffer(const Buffer& other);

  // dangerous raw pointer constructor
  // data pointer assumed to be stored on heap with correct size and capacity args
  Buffer(T* data, size_t size, size_t capacity);

  // assignment operator
  Buffer& operator=(const Buffer& other);

  // get current capacity of underlying array
  size_t capacity();

  // manually resize capacity of buffer, true if successful
  bool resize(size_t capacity);

  // number of elements in buffer
  size_t size();

  // add element to the end of the list of elements, no check on capacity
  void append(T data);

  // append with capacity check, amortized doubling used to resize buffer if needed
  void safe_append(T data);

  // access underlying array
  T * data();

  // access operator
  T& operator[](int index);

  // iterator
  typedef T * iterator;
  typedef const T * const_iterator;
  iterator begin() { return &_data[0]; }
  iterator end() { return &_data[_size]; }
};

typedef Buffer<unsigned char> ByteBuffer;

}  // namespace dsa

#endif  // DSA_SDK_UTIL_BUFFER_H_