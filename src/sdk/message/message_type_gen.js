const fs = require('fs');


const messages = {
  "Request":{
    "Invoke":{
      "Body":true,
      "Priority":true,
      "Status":false,
      "SequenceId":true,
      "PageId":true,
      "AliasCount":true,
      "TargetPath":true,
      "PermissionToken":true,
      "MaxPermission":true,
      "NoStream":true,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":false,
      "SourcePath":false,
      "Skippable":false,
      
    },
    "List":{
      "Body":false,
      "Priority":true,
      "Status":false,
      "SequenceId":false,
      "PageId":false,
      "AliasCount":true,
      "TargetPath":true,
      "PermissionToken":true,
      "MaxPermission":false,
      "NoStream":true,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":false,
      "SourcePath":false,
      "Skippable":false,
      
    },
    "Set":{
      "Body":true,
      "Priority":true,
      "Status":false,
      "SequenceId":false,
      "PageId":true,
      "AliasCount":true,
      "TargetPath":true,
      "PermissionToken":true,
      "MaxPermission":false,
      "NoStream":false,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":false,
      "SourcePath":false,
      "Skippable":false,
      
    },
    "Subscribe":{
      "Body":false,
      "Priority":true,
      "Status":false,
      "SequenceId":false,
      "PageId":false,
      "AliasCount":true,
      "TargetPath":true,
      "PermissionToken":true,
      "MaxPermission":false,
      "NoStream":true,
      "Qos":true,
      "QueueSize":true,
      "QueueTime":true,
      "BasePath":false,
      "SourcePath":false,
      "Skippable":false,
      
    }
  },
  "Response":{
    "Invoke":{
      "Body":true,
      "Priority":false,
      "Status":true,
      "SequenceId":true,
      "PageId":true,
      "AliasCount":false,
      "TargetPath":false,
      "PermissionToken":false,
      "MaxPermission":false,
      "NoStream":false,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":false,
      "SourcePath":false,
      "Skippable":true,
      
    },
    "List":{
      "Body":true,
      "Priority":false,
      "Status":true,
      "SequenceId":true,
      "PageId":false,
      "AliasCount":false,
      "TargetPath":false,
      "PermissionToken":false,
      "MaxPermission":false,
      "NoStream":false,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":true,
      "SourcePath":true,
      "Skippable":false,
      
    },
    "Set":{
      "Body":false,
      "Priority":false,
      "Status":true,
      "SequenceId":false,
      "PageId":false,
      "AliasCount":false,
      "TargetPath":false,
      "PermissionToken":false,
      "MaxPermission":false,
      "NoStream":false,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":false,
      "SourcePath":false,
      "Skippable":false,
      
    },
    "Subscribe":{
      "Body":true,
      "Priority":false,
      "Status":true,
      "SequenceId":true,
      "PageId":true,
      "AliasCount":false,
      "TargetPath":false,
      "PermissionToken":false,
      "MaxPermission":false,
      "NoStream":false,
      "Qos":false,
      "QueueSize":false,
      "QueueTime":false,
      "BasePath":false,
      "SourcePath":true,
      "Skippable":false,
      
    }
  }
};

function gen_source(path, typename, baseTypeName, header, configs) {

let data = `#include "dsa_common.h"

#include "${header}"`;
data+=`

namespace dsa {
`;

    data+=`
${typename}::${typename}(const ${typename}& from)
    : ${baseTypeName}(from.static_headers) {`;    
    if (configs.Priority) data+=` 
  if (from.priority != nullptr) {
     priority.reset(new DynamicBoolHeader(DynamicHeader::Priority));
  }`;    
    if (configs.Status) data+=` 
  if (from.status != nullptr) {
    status.reset(new DynamicByteHeader(DynamicHeader::Status, from.status->value()));
  }`;    
    if (configs.SequenceId) data+=` 
  if (from.sequence_id != nullptr) {
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SequenceId, from.sequence_id->value()));
  }`;    
    if (configs.PageId) data+=` 
  if (from.page_id != nullptr) {
    page_id.reset(new DynamicIntHeader(DynamicHeader::PageId, from.page_id->value()));
  }`;    
    if (configs.AliasCount) data+=` 
  if (from.alias_count != nullptr) {
    alias_count.reset(new DynamicByteHeader(DynamicHeader::AliasCount, from.alias_count->value()));
  }`;    
    if (configs.PermissionToken) data+=` 
  if (from.permission_token != nullptr) {
    permission_token.reset(new DynamicStringHeader(DynamicHeader::PermissionToken, from.permission_token->value()));
  }`;    
    if (configs.MaxPermission) data+=` 
  if (from.max_permission != nullptr) {
    max_permission.reset(new DynamicByteHeader(DynamicHeader::MaxPermission, from.max_permission->value()));
  }`;
      if (configs.TargetPath) data+=` 
  if (from.target_path != nullptr) {
    target_path.reset(new DynamicStringHeader(DynamicHeader::TargetPath, from.target_path->value()));
  }`;   
    if (configs.NoStream) data+=` 
  if (from.no_stream != nullptr) {
    no_stream.reset(new DynamicBoolHeader(DynamicHeader::NoStream));
  }`;    
    if (configs.Qos) data+=` 
  if (from.qos != nullptr) {
    qos.reset(new DynamicByteHeader(DynamicHeader::Qos, from.qos->value()));
  }`;    
    if (configs.QueueSize) data+=` 
  if (from.queue_size != nullptr) {
    queue_size.reset(new DynamicIntHeader(DynamicHeader::QueueSize, from.queue_size->value()));
  }`;    
    if (configs.QueueTime) data+=` 
  if (from.queue_time != nullptr) {
    queue_time.reset(new DynamicIntHeader(DynamicHeader::QueueTime, from.queue_time->value()));
  }`;     
    if (configs.BasePath) data+=` 
  if (from.base_path != nullptr) {
    base_path.reset(new DynamicStringHeader(DynamicHeader::BasePath, from.base_path->value()));
  }`;    
    if (configs.SourcePath) data+=` 
  if (from.source_path != nullptr) {
    source_path.reset(new DynamicStringHeader(DynamicHeader::SourcePath, from.source_path->value()));
  }`;    
    if (configs.Skippable) data+=` 
  if (from.skippable != nullptr) {
    skippable.reset(new DynamicBoolHeader(DynamicHeader::Skippable));
  }`;
    if (configs.Body) data+=` 
  if (from.body != nullptr) {
    body.reset(new SharedBuffer(*from.body));
  }`;
    data+=`
}

void ${typename}::parse_dynamic_headers(const uint8_t* data, size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    uint8_t key = header->key();`;
    if (configs.Priority) data+=`;
    if (key == DynamicHeader::Priority) {
      priority.reset(static_cast<DynamicBoolHeader*>(header));
    }`;
    if (configs.Status) data+=`
    if (key == DynamicHeader::Status) {
      status.reset(static_cast<DynamicByteHeader*>(header));
    }`; 
    if (configs.SequenceId) data+=` else if (key == DynamicHeader::SequenceId) {
      sequence_id.reset(static_cast<DynamicIntHeader*>(header));
    }`;
    if (configs.PageId) data+=` else if (key == DynamicHeader::PageId) {
      page_id.reset(static_cast<DynamicIntHeader*>(header));
    }`;
    if (configs.AliasCount) data+=` else if (key == DynamicHeader::AliasCount) {
      alias_count.reset(static_cast<DynamicByteHeader*>(header));
    }`;
    if (configs.TargetPath) data+=` else if (key == DynamicHeader::TargetPath) {
      target_path.reset(static_cast<DynamicStringHeader*>(header));
    }`;
    if (configs.PermissionToken) data+=` else if (key == DynamicHeader::PermissionToken) {
      permission_token.reset(static_cast<DynamicStringHeader*>(header));
    }`;
    if (configs.MaxPermission) data+=` else if (key == DynamicHeader::MaxPermission) {
      max_permission.reset(static_cast<DynamicByteHeader*>(header));
    }`;
    if (configs.NoStream) data+=` else if (key == DynamicHeader::NoStream) {
      no_stream.reset(static_cast<DynamicBoolHeader*>(header));
    }`;
    if (configs.Qos) data+=` else if (key == DynamicHeader::Qos) {
      qos.reset(static_cast<DynamicByteHeader*>(header));
    }`;
    if (configs.QueueSize) data+=` else if (key == DynamicHeader::QueueSize) {
      queue_size.reset(static_cast<DynamicIntHeader*>(header));
    }`;
    if (configs.QueueTime) data+=` else if (key == DynamicHeader::QueueTime) {
      queue_time.reset(static_cast<DynamicIntHeader*>(header));
    }`;   
    if (configs.BasePath) data+=` else if (key == DynamicHeader::BasePath) {
      base_path.reset(static_cast<DynamicStringHeader*>(header));
    }`;    
    if (configs.SourcePath) data+=` else if (key == DynamicHeader::SourcePath) {
      source_path.reset(static_cast<DynamicStringHeader*>(header));
    }`;    
    if (configs.Skippable) data+=` else if (key == DynamicHeader::Skippable) {
      skippable.reset(static_cast<DynamicBoolHeader*>(header));
    }`;    
    data+=`
  }
}

void ${typename}::write_dynamic_data(uint8_t* data) const {`;    
    if (configs.Priority) data+=` 
  if (priority != nullptr) {
    priority->write(data);
    data += priority->size();
  }`;    
    if (configs.Status) data+=` 
  if (status != nullptr) {
    status->write(data);
    data += status->size();
  }`;    
    if (configs.SequenceId) data+=` 
  if (sequence_id != nullptr) {
    sequence_id->write(data);
    data += sequence_id->size();
  }`;    
    if (configs.PageId) data+=` 
  if (page_id != nullptr) {
    page_id->write(data);
    data += page_id->size();
  }`;    
    if (configs.AliasCount) data+=` 
  if (alias_count != nullptr) {
    alias_count->write(data);
    data += alias_count->size();
  }`;    
    if (configs.PermissionToken) data+=` 
  if (permission_token != nullptr) {
    permission_token->write(data);
    data += permission_token->size();
  }`;    
    if (configs.MaxPermission) data+=` 
  if (max_permission != nullptr) {
    max_permission->write(data);
    data += max_permission->size();
  }`;
      if (configs.TargetPath) data+=` 
  if (target_path != nullptr) {
    target_path->write(data);
    data += target_path->size();
  }`;   
    if (configs.NoStream) data+=` 
  if (no_stream != nullptr) {
    no_stream->write(data);
    data += no_stream->size();
  }`;    
    if (configs.Qos) data+=` 
  if (qos != nullptr) {
    qos->write(data);
    data += qos->size();
  }`;    
    if (configs.QueueSize) data+=` 
  if (queue_size != nullptr) {
    queue_size->write(data);
    data += queue_size->size();
  }`;    
    if (configs.QueueTime) data+=` 
  if (queue_time != nullptr) {
    queue_time->write(data);
    data += queue_time->size();
  }`;     
    if (configs.BasePath) data+=` 
  if (base_path != nullptr) {
    base_path->write(data);
    data += base_path->size();
  }`;    
    if (configs.SourcePath) data+=` 
  if (source_path != nullptr) {
    source_path->write(data);
    data += source_path->size();
  }`;    
    if (configs.Skippable) data+=` 
  if (skippable != nullptr) {
    skippable->write(data);
    data += skippable->size();
  }`;
    if (configs.Body) data+=` 
  if (body != nullptr) {
    memcpy(data, body->data, body->size);
  }`;
    data+=`
}

void ${typename}::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize;`;
    if (configs.Priority) data+=` 
  if (priority != nullptr) {
    header_size += priority->size();
  }`;
    if (configs.Status) data+=` 
  if (status != nullptr) {
    header_size += status->size();
  }`;
    if (configs.SequenceId) data+=` 
  if (sequence_id != nullptr) {
    header_size += sequence_id->size();
  }`;
    if (configs.PageId) data+=` 
  if (page_id != nullptr) {
    header_size += page_id->size();
  }`;
    if (configs.AliasCount) data+=` 
  if (alias_count != nullptr) {
    header_size += alias_count->size();
  }`;
    if (configs.TargetPath) data+=` 
  if (target_path != nullptr) {
    header_size += target_path->size();
  }`;
    if (configs.PermissionToken) data+=` 
  if (permission_token != nullptr) {
    header_size += permission_token->size();
  }`;
    if (configs.MaxPermission) data+=` 
  if (max_permission != nullptr) {
    header_size += max_permission->size();
  }`;
    if (configs.NoStream) data+=` 
  if (no_stream != nullptr) {
    header_size += no_stream->size();
  }`;
    if (configs.Qos) data+=` 
  if (qos != nullptr) {
    header_size += qos->size();
  }`;
    if (configs.QueueSize) data+=` 
  if (queue_size != nullptr) {
    header_size += queue_size->size();
  }`;
    if (configs.QueueTime) data+=` 
  if (queue_time != nullptr) {
    header_size += queue_time->size();
  }`;
    if (configs.BasePath) data+=` 
  if (base_path != nullptr) {
    header_size += base_path->size();
  }`;
    if (configs.SourcePath) data+=` 
  if (source_path != nullptr) {
    header_size += source_path->size();
  }`;
    if (configs.Skippable) data+=` 
  if (skippable != nullptr) {
    header_size += skippable->size();
  }`;
    data+=`

  uint32_t message_size = header_size;`;
    if (configs.Body) data+=` 
  if (body != nullptr) {
    message_size += body->size;
  }`;
    data+=`
  static_headers.message_size = message_size;
  static_headers.header_size = header_size;
}

} // namespace dsa
`;
fs.writeFileSync(path, data);

}

for (type in messages){
  let type_l = type.toLowerCase();
  for (method in messages[type]){
    let method_l = method.toLowerCase();
    let configs = messages[type][method];
    gen_source(
      `${type_l}/${method_l}_${type_l}_message_generated.cc`, 
      `${method}${type}Message`,
      `${type}Message`,
      `${method_l}_${type_l}_message.h`,
      configs
    );
  }
}

