#include "buffer_factory.h"

namespace dsa {

Buffer *BufferFactory::get_buffer() {
  Buffer *out;
  if (_queue.try_dequeue(out))
    return out;
  BufferPtr new_buf = std::make_shared<Buffer>();
  out = new_buf.get();
  _buffers.enqueue(new_buf);
  return out;
}

bool BufferFactory::recycle(Buffer *in) {
  return _queue.enqueue(in);
}

}  // namespace dsa