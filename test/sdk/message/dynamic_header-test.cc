#include "dsa/message.h"
#include "gtest/gtest.h"

#include <typeinfo>

using namespace dsa;

enum DynamicHeaderType : uint8_t {
  STRING_HEADER,
  BYTE_HEADER,
  INT_HEADER,
  BOOL_HEADER
};

struct TestSpec {
  DynamicHeader::DynamicKey key;
  DynamicHeaderType expected_dynamic_header_type;
};


TEST(DynamicHeaderTest, parse) {

  TestSpec test_spec[] = {
    { DynamicHeader::STATUS, BYTE_HEADER },
    { DynamicHeader::SEQUENCE_ID, INT_HEADER },
    { DynamicHeader::PAGE_ID, INT_HEADER },
    { DynamicHeader::ALIAS_COUNT, BYTE_HEADER },
    { DynamicHeader::PRIORITY, BOOL_HEADER },
    { DynamicHeader::NO_STREAM, BOOL_HEADER },
    { DynamicHeader::QOS, BYTE_HEADER },
    { DynamicHeader::QUEUE_SIZE, INT_HEADER },
    { DynamicHeader::QUEUE_TIME, INT_HEADER },
    { DynamicHeader::BASE_PATH, STRING_HEADER },
    { DynamicHeader::SKIPPABLE, BOOL_HEADER },
    { DynamicHeader::MAX_PERMISSION, BYTE_HEADER },
    { DynamicHeader::ATTRIBUTE_FIELD, STRING_HEADER },
    { DynamicHeader::PERMISSION_TOKEN, STRING_HEADER },
    { DynamicHeader::TARGET_PATH, STRING_HEADER },
    { DynamicHeader::SOURCE_PATH, STRING_HEADER }
  };

  uint8_t test_spec_size = sizeof(test_spec)/sizeof(TestSpec);
  for (size_t i = 0; i < test_spec_size; ++i) {
    uint8_t data[8];
    data[0] = test_spec[i].key;
    data[1] = 5;
    data[2] = 0x0;
    data[3] = 'p';
    data[4] = 'a';
    
    DynamicHeader* dynamic_header = DynamicHeader::parse(data, 10);

    switch (test_spec[i].expected_dynamic_header_type) {
    case STRING_HEADER:
      EXPECT_TRUE((typeid(*dynamic_header) == typeid(DynamicStringHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicByteHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicIntHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicBoolHeader)));
      break;
    case BYTE_HEADER:
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicStringHeader)));
      EXPECT_TRUE((typeid(*dynamic_header) == typeid(DynamicByteHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicIntHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicBoolHeader)));
      break;
    case INT_HEADER:
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicStringHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicByteHeader)));
      EXPECT_TRUE((typeid(*dynamic_header) == typeid(DynamicIntHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicBoolHeader)));
      break;
    case BOOL_HEADER:
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicStringHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicByteHeader)));
      EXPECT_FALSE((typeid(*dynamic_header) == typeid(DynamicIntHeader)));
      EXPECT_TRUE((typeid(*dynamic_header) == typeid(DynamicBoolHeader)));
      break;
    default:
      ;
    }
    delete dynamic_header;
  }
}

