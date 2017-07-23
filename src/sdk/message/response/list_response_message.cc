#include "list_response_message.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parseDynamicHeaders(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size() - StaticHeaders::TotalSize);
}

void ListResponseMessage::parseDynamicHeaders(const uint8_t* data,
                                              size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    uint8_t key = header->key();

    if (key == DynamicHeader::Priority) {
      priority.reset(static_cast<DynamicByteHeader*>(header));
    }  //
    else if (key == DynamicHeader::Status) {
      status.reset(static_cast<DynamicByteHeader*>(header));
    }  //
    // else if (key == DynamicHeader::SequenceId) {
    //  sequence_id.reset(static_cast<DynamicIntHeader*>(header));
    //} //
    // else if (key == DynamicHeader::PageId) {
    //  page_id.reset(static_cast<DynamicIntHeader*>(header));
    //} //
    // else if (key == DynamicHeader::AliasCount) {
    //  alias_count.reset(static_cast<DynamicByteHeader*>(header));
    //} //
    // else if (key == DynamicHeader::TargetPath) {
    //  target_path.reset(static_cast<DynamicStringHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::PermissionToken) {
    //  permission_token.reset(static_cast<DynamicStringHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::MaxPermission) {
    //  max_permission_token.reset(static_cast<DynamicByteHeader*>(header));
    //} //
    // else if (key == DynamicHeader::NoStream) {
    //  no_stream.reset(static_cast<DynamicBoolHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::Qos) {
    //  qos.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::QueueSize) {
    //  queue_size.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::QueueTime) {
    //  queue_time.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::UpdateFrequency) {
    //  update_frequency.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    else if (key == DynamicHeader::BasePath) {
      base_path.reset(static_cast<DynamicStringHeader*>(header));
    }  //
    else if (key == DynamicHeader::SourcePath) {
      source_path.reset(static_cast<DynamicStringHeader*>(header));
    }  //
    // else if (key == DynamicHeader::Skippable) {
    //  skippable.reset(static_cast<DynamicBoolHeader*>(header));
    //} //
  }
}

}  // namespace dsa
