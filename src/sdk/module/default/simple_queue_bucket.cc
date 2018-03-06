#include "dsa_common.h"

#include "simple_storage.h"

namespace dsa {

void SimpleQueueBucket::push_back(const wstring_& key, BytesRef&& data) {
  return;
}

// when count = 0, remove all elements in the queue
void SimpleQueueBucket::remove_front(const wstring_& key, size_t count) {
  return;
}

void SimpleQueueBucket::read_all(ReadCallback&& callback,
                                 std::function<void()>&& on_done) {
  return;
}

void SimpleQueueBucket::remove_all() { return; }

}  // namespace dsa
