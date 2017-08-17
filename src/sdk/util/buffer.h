#ifndef DSA_SDK_UTIL_BUFFER_H_
#define DSA_SDK_UTIL_BUFFER_H_

#include <iostream>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "enable_intrusive.h"

namespace dsa {
class Message;

class ByteBuffer : public std::vector<uint8_t>,
                   public EnableIntrusive<ByteBuffer> {
 public:
  template <typename... Args>
  inline explicit ByteBuffer(Args &&... args)
      : std::vector<uint8_t>(std::forward<Args>(args)...){};

  typedef std::vector<uint8_t>::iterator iterator;
  typedef std::vector<uint8_t>::const_iterator const_iterator;
};

typedef intrusive_ptr_<ByteBuffer> BufferPtr;
}  // namespace dsa

inline std::ostream &operator<<(std::ostream &os, const dsa::ByteBuffer &buf) {
  std::stringstream ss;
  ss << "[";
  if (buf.size() > 0) {
    for (unsigned int i = 0; i < buf.size() - 1; ++i) {
      ss << std::hex << (unsigned int) (buf[i]) << std::dec << ", ";
    }
    ss << std::hex << (unsigned int) (buf[buf.size() - 1]) << std::dec;
  }
  ss << "]";
  return os << ss.str();
}

//class Buffer : public std::vector<uint8_t>, public EnableIntrusive<Buffer> {
// public:
// private:
//  enum { default_capacity = 256 };
//  uint8_t *_data;
//  size_t _size;
//  size_t _capacity;
//
// public:
//  class SharedBuffer {
//   private:
//    intrusive_ptr_<Buffer> _parent_buf;
//
//   public:
//    const size_t size;
//    const uint8_t * data;
//
//    SharedBuffer(intrusive_ptr_<Buffer> parent, const uint8_t * data, size_t size)
//        : _parent_buf(std::move(parent)), data(data), size(size) {}
//
//    SharedBuffer(const SharedBuffer &buf) : _parent_buf(buf._parent_buf), size(buf.size), data(buf.data) {}
//
//    SharedBuffer(SharedBuffer &&buf) : _parent_buf(std::move(buf._parent_buf)), size(buf.size), data(buf.data) {}
//  };
//
//  // default constructor
//  Buffer();
//
//  // deconstructor
//  ~Buffer();
//
//  // set capacity constructor
//  explicit Buffer(size_t capacity);
//
//  // shallow copy constructor
//  Buffer(const Buffer &other);
//
//  // string copy constructor
//  explicit Buffer(std::string data);
//
//  // dangerous raw pointer constructor
//  // data pointer assumed to be stored on heap with correct size and capacity args
//  Buffer(uint8_t *data, size_t size, size_t capacity);
//
//  // assignment operator
//  Buffer &operator=(const Buffer &other) throw(const std::runtime_error &);
//
//  // get current capacity of underlying array
//  size_t capacity() const;
//
//  // manually resize capacity of buffer, true if successful
//  bool resize(size_t capacity) throw(const std::runtime_error &);
//
//  // number of elements in buffer
//  size_t size() const;
//
//  // append message contents to buffer data
//  void append(const Message &message);
//
//  // add element to the end of the list of elements, no check on capacity
//  void append(uint8_t data);
//
//  // append with capacity check, amortized doubling used to resize buffer if needed
//  void safe_append(uint8_t data);
//
//  // copy from pointer `size` number of items
//  void assign(const uint8_t *data, size_t size) throw(const std::runtime_error &);
//
//  // access underlying array
//  uint8_t *data();
//
//  // access underlying constant data
//  const uint8_t *data() const;
//
//  // get buffer
//  SharedBuffer get_shared_buffer(size_t offset, size_t size);
//
//  std::string to_string() { return std::move(std::string(reinterpret_cast<char *>(_data), _size)); }
//
//  //////////////////////////
//  // Operators
//  //////////////////////////
//  // access operator
//  uint8_t &operator[](size_t index) throw(const std::runtime_error &);
//
//  // const access operator
//  const uint8_t &operator[](size_t index) const throw(const std::runtime_error &);
//
//  // equals operator
//  bool operator==(const Buffer &other) const;
//
//  // not equals operator
//  bool operator!=(const Buffer &other) const { return !(*this == other); }
//
//  //////////////////////////
//  // Iterator
//  //////////////////////////
//  typedef uint8_t *iterator;
//  typedef const uint8_t *const_iterator;
//  iterator begin() { return &_data[0]; }
//  iterator end() { return &_data[_size]; }
//};

//typedef intrusive_ptr_<Buffer> BufferPtr;
//typedef intrusive_ptr_<const Buffer> ConstBufferPtr;
//typedef intrusive_ptr_<Buffer::SharedBuffer> MessageBufferPtr;
//typedef Buffer::SharedBuffer SharedBuffer;
//typedef Buffer::SharedBuffer ValueUpdate;

#endif  // DSA_SDK_UTIL_BUFFER_H_