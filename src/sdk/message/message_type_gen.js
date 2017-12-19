const fs = require('fs');

function camelToUnder(toFormat) {
    // copy string as to not change original
    let str = toFormat.slice(0);

    // define change function
    function changeAt(idx) {
        str = str.slice(0, idx) + '_' + str.charAt(idx).toLowerCase() +
            str.slice(idx + 1)
    }

    // do change at uppercase
    if (str.length >= 1) {
        str = str.charAt(0).toLowerCase() + str.slice(1)
    }
    for (let i = 1; i < str.length; i += 1) {
        if (str.charAt(i).toUpperCase() === str.charAt(i)) {
            changeAt(i)
        }
    }
    return str
}

class Header {
    constructor(name) {
        this.name = name;
        this.underName = camelToUnder(name);
        this.upperName = this.underName.toUpperCase();
    }

    writeDynamicDataStatement() {
        return `
  if (${this.underName} != nullptr) {
    ${this.underName}->write(data);
    data += ${this.underName}->size();
  }`
    }

    updateStaticHeadersStatement() {
        return `
  if (${this.underName} != nullptr) {
    header_size += ${this.underName}->size();
  }`
    }

    updatePrintHeadersStatement() {
        return `
  if (${this.underName} != nullptr) {
    os << " ${this.name}: " << ${this.underName}->value();
  }`
    }
}

class BoolHeader extends Header {
    copyConstructorStatement() {
        return `new DynamicBoolHeader(DynamicHeader::${this.upperName})`
    }

    parseDynamicHeadersStatement() {
        return `
      case DynamicHeader::${this.upperName}:${this.underName}.reset(DOWN_CAST<DynamicBoolHeader *>(header.release()));
        break;`
    }

    updatePrintHeadersStatement() {
        return `
  if (${this.underName} != nullptr) {
    os << " ${this.name}";
  }`
    }
}

class IntHeader extends Header {
    copyConstructorStatement() {
        return `new DynamicIntHeader(DynamicHeader::${this.upperName}, from.${this.underName}->value())`
    }

    parseDynamicHeadersStatement() {
        return `
      case DynamicHeader::${this.upperName}:${this.underName}.reset(DOWN_CAST<DynamicIntHeader *>(header.release()));
        break;`
    }
}

class StringHeader extends Header {
    copyConstructorStatement() {
        return `new DynamicStringHeader(DynamicHeader::${this.upperName}, from.${this.underName}->value())`
    }

    parseDynamicHeadersStatement() {
        return `
      case DynamicHeader::${this.upperName}:${this.underName}.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        break;`
    }
}

class ByteHeader extends Header {
    copyConstructorStatement() {
        return `new DynamicByteHeader(DynamicHeader::${this.upperName}, from.${this.underName}->value())`
    }

    parseDynamicHeadersStatement() {
        return `
      case DynamicHeader::${this.upperName}:${this.underName}.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
        break;`
    }

    updatePrintHeadersStatement() {
        return `
  if (${this.underName} != nullptr) {
    os << " ${this.name}: x" << std::hex << int(${this.underName}->value()) << std::dec;
  }`
    }
}

class BodyHeader extends Header {
    copyConstructorStatement() {
        return `from.${this.underName}.get()`
    }

    writeDynamicDataStatement() {
        return `
  if (body != nullptr) {
    std::copy(body->begin(), body->end(), data);
  }`
    }

    updateStaticHeadersStatement() {
        return `
  if (body != nullptr) {
    message_size += body->size();
  }`
    }
}

class TargetPathHeader extends StringHeader {
    parseDynamicHeadersStatement() {
        return `
      case DynamicHeader::${this.upperName}: {
        ${this.underName}.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        _parsed_target_path.reset(new Path(${this.underName}->value()));
        break;
      }`
    }
}

const messages = {
    Request: {
        Invoke: [
            new BoolHeader('Priority'),
            new IntHeader('SequenceId'),
            new IntHeader('PageId'),
            new ByteHeader('AliasCount'),
            new TargetPathHeader('TargetPath'),
            new StringHeader('PermissionToken'),
            new ByteHeader('MaxPermission'),
            new BoolHeader('NoStream'),
            new BoolHeader('Refreshed'),
            new BoolHeader('Skippable'),
            new BodyHeader('Body'),
        ],
        List: [
            new ByteHeader('AliasCount'),
            new TargetPathHeader('TargetPath'),
            new StringHeader('PermissionToken'),
            new BoolHeader('NoStream'),
        ],
        Set: [
            new BoolHeader('Priority'),
            new IntHeader('PageId'),
            new ByteHeader('AliasCount'),
            new TargetPathHeader('TargetPath'),
            new StringHeader('PermissionToken'),
            new BoolHeader('NoStream'),
            new StringHeader('AttributeField'),
            new BodyHeader('Body'),
        ],
        Subscribe: [
            new BoolHeader('Priority'),
            new ByteHeader('AliasCount'),
            new TargetPathHeader('TargetPath'),
            new StringHeader('PermissionToken'),
            new BoolHeader('NoStream'),
            new ByteHeader('Qos'),
            new IntHeader('QueueSize'),
            new IntHeader('QueueTime'),
        ]
    },
    Response: {
        Invoke: [
            new ByteHeader('Status'),
            new IntHeader('SequenceId'),
            new IntHeader('PageId'),
            new BoolHeader('Refreshed'),
            new BoolHeader('Skippable'),
            new BodyHeader('Body'),
        ],
        List: [
            new ByteHeader('Status'),
            new BoolHeader('Refreshed'),
            new IntHeader('SequenceId'),
            new StringHeader('PubPath'),
            new StringHeader('SourcePath'),
            new BodyHeader('Body'),
        ],
        Set: [
            new ByteHeader('Status'),
        ],
        Subscribe: [
            new ByteHeader('Status'),
            new IntHeader('SequenceId'),
            new IntHeader('PageId'),
            new StringHeader('SourcePath'),
            new BodyHeader('Body'),
        ]
    }
};

function gen_source(path, typename, baseTypeName, header, configs) {
    let hasBody = false;
    let data = `#include "dsa_common.h"

#include "${header}"`;
    data += `

#include <iostream>

namespace dsa {
`;

    data += `
${typename}::${typename}(const ${typename}& from)
    : ${baseTypeName}(from.static_headers) {`;
    configs.forEach(field => {
        data += `
  if (from.${field.underName} != nullptr)
    ${field.underName}.reset(${field.copyConstructorStatement()});`
    });
    data += `
}

void ${typename}::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {`;

    configs.forEach(field => {
        if (field.name !== 'Body') {
            data += field.parseDynamicHeadersStatement()
        } else {
            hasBody = true;
        }
    });
    data += `
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }`;
    if (hasBody) {
        if (typename == 'ListResponseMessage') {
            data+=`
  if ( body_size > 0) {
      body.reset(new RefCountBytes(data, data + body_size));
      parse(); // parse the map right after decoding
  }`;
        } else {
            data+=`
  if ( body_size > 0) {
      body.reset(new RefCountBytes(data, data + body_size));
  }`;
        }

    }

    data+=`
}

void ${typename}::write_dynamic_data(uint8_t *data) const {`;
    configs.forEach(field => {
        if (field.name !== 'Body')
            data += field.writeDynamicDataStatement();

    });
    if (hasBody) data += new BodyHeader('Body').writeDynamicDataStatement()
    data += `
}

void ${typename}::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;`;
    configs.forEach(field => {
        if (field.name !== 'Body')
            data += field.updateStaticHeadersStatement();

    });
    data += `

  uint32_t message_size = header_size;`;
    if (hasBody) data += ` 
  if (body != nullptr) {
    message_size += body->size();
  }`;
    data += `
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

void ${typename}::print_headers(std::ostream &os) const {
`;
    configs.forEach(field => {
        if (field.name !== 'Body')
            data += field.updatePrintHeadersStatement();
    });
    data += `
}

}  // namespace dsa
`;
    // console.log(`// ${path}\n${data}`)
    fs.writeFileSync(path, data);
}

Object.keys(messages).forEach(type => {
    let type_l = type.toLowerCase();
    Object.keys(messages[type]).forEach(method => {
        let method_l = method.toLowerCase();
        let configs = messages[type][method];
        gen_source(
            `${type_l}/${method_l}_${type_l}_message_generated.cc`,
            `${method}${type}Message`,
            `${type}Message`,
            `${method_l}_${type_l}_message.h`,
            configs
        );
    });
});

