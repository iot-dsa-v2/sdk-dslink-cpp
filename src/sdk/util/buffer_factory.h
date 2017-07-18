#ifndef  DSA_SDK_UTIL_BUFFER_FACTORY_H_
#define  DSA_SDK_UTIL_BUFFER_FACTORY_H_

#include <queue>

#include "concurrentqueue.h"

#include "buffer.h"

namespace dsa {

class BufferFactory {
 private:
  moodycamel::ConcurrentQueue<Buffer *> _queue;
  moodycamel::ConcurrentQueue<BufferPtr> _buffers;

 public:
  Buffer *get_buffer();

  bool recycle(Buffer *);
};

}  // namespace dsa

#endif  // DSA_SDK_UTIL_BUFFER_FACTORY_H_
