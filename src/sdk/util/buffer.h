#ifndef DSA_SDK_UTIL_BUFFER_H_
#define DSA_SDK_UTIL_BUFFER_H_

#include <iostream>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace dsa {
class Message;

class Buffer : public std::enable_shared_from_this<Buffer> {
 private:
  enum { default_capacity = 256 };
  uint8_t *_data;
  size_t _size;
  size_t _capacity;

 public:
  class SharedBuffer {
   private:
    std::shared_ptr<Buffer> _parent_buf;
   public:
    const size_t size;
    const uint8_t * data;

    SharedBuffer(std::shared_ptr<Buffer> parent, const uint8_t * data, size_t size)
        : _parent_buf(std::move(parent)), data(data), size(size) {}

    SharedBuffer(const SharedBuffer &buf) : _parent_buf(buf._parent_buf), size(buf.size), data(buf.data) {}
  };

  // default constructor
  Buffer();

  // deconstructor
  ~Buffer();

  // set capacity constructor
  explicit Buffer(size_t capacity);

  // shallow copy constructor
  Buffer(const Buffer &other);

  // string copy constructor
  explicit Buffer(std::string data);

  // dangerous raw pointer constructor
  // data pointer assumed to be stored on heap with correct size and capacity args
  Buffer(uint8_t *data, size_t size, size_t capacity);

  // assignment operator
  Buffer &operator=(const Buffer &other);

  // get current capacity of underlying array
  size_t capacity() const;

  // manually resize capacity of buffer, true if successful
  bool resize(size_t capacity);

  // number of elements in buffer
  size_t size() const;

  // append message contents to buffer data
  void append(const Message &message);

  // add element to the end of the list of elements, no check on capacity
  void append(uint8_t data);

  // append with capacity check, amortized doubling used to resize buffer if needed
  void safe_append(uint8_t data);

  // copy from pointer `size` number of items
  void assign(const uint8_t *data, size_t size);

  // access underlying array
  uint8_t *data();

  // access underlying constant data
  const uint8_t *data() const;

  // access operator
  uint8_t &operator[](size_t index);

  // const access operator
  const uint8_t &operator[](size_t index) const;

  // get buffer
  SharedBuffer get_shared_buffer(size_t offset, size_t size);

  std::string to_string() { return std::move(std::string(reinterpret_cast<char *>(_data), _size)); }

  // iterator
  typedef uint8_t *iterator;
  typedef const uint8_t *const_iterator;
  iterator begin() { return &_data[0]; }
  iterator end() { return &_data[_size]; }
};

typedef std::shared_ptr<Buffer> BufferPtr;
typedef std::shared_ptr<const Buffer> ConstBufferPtr;
typedef std::shared_ptr<Buffer::SharedBuffer> MessageBufferPtr;
typedef Buffer::SharedBuffer SharedBuffer;
typedef Buffer::SharedBuffer ValueUpdate;

}  // namespace dsa

inline std::ostream &operator<<(std::ostream &os, const dsa::Buffer &buf) {
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

#endif  // DSA_SDK_UTIL_BUFFER_H_