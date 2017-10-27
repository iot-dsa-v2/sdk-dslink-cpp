#include "simple_storage.h"

namespace dsa {

void SimpleQueueBucket::push_back(const std::string& key, BytesRef&& data) {
  return;
}

// when count = 0, remove all elements in the queue
void SimpleQueueBucket::remove_front(const std::string& key, size_t count) {
  return;
}

void SimpleQueueBucket::read_all(ReadCallback&& callback,
	      std::function<void()>&& on_done) {
  return;
}

void SimpleQueueBucket::remove_all() {
  return;
}

}  // namespace dsa
