#include "subscribe_request_message.h"

namespace dsa {
SubscribeRequestMessage::SubscribeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parseDynamicHeaders(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size() - StaticHeaders::TotalSize);
}
void SubscribeRequestMessage::parseDynamicHeaders(const uint8_t* data,
                                                  size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    switch (header->key()) {
      case DynamicHeader::Priority:
        priority.reset(static_cast<DynamicByteHeader*>(header));
        break;
      case DynamicHeader::TargetPath:
        target_path.reset(static_cast<DynamicStringHeader*>(header));
        break;
      case DynamicHeader::PermissionToken:
        permission_token.reset(static_cast<DynamicStringHeader*>(header));
        break;
      case DynamicHeader::NoStream:
        no_stream.reset(static_cast<DynamicBoolHeader*>(header));
        break;
      case DynamicHeader::Qos:
        qos.reset(static_cast<DynamicByteHeader*>(header));
        break;
      case DynamicHeader::QueueSize:
        queue_size.reset(static_cast<DynamicByteHeader*>(header));
        break;
      case DynamicHeader::QueueTime:
        queue_time.reset(static_cast<DynamicByteHeader*>(header));
        break;
      case DynamicHeader::UpdateFrequency:
        update_frequency.reset(static_cast<DynamicByteHeader*>(header));
        break;
      default:
        delete header;
        // TODO: throw error on unexpected header?
        break;
    }
  }
}
}  // namespace dsa
