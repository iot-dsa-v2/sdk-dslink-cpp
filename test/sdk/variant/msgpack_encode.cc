#include "dsa/variant.h"
#include "gtest/gtest.h"

#include <msgpack.h>

using namespace dsa;

TEST(VariantTest, MsgpackEncoding) {
  Variant v(123);

  std::vector<uint8_t> *encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg->size();

  Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash->is_int());
  EXPECT_EQ(123, v_dash->get_int());
}


