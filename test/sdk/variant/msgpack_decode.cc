#include "dsa/variant.h"
#include "gtest/gtest.h"

#include <msgpack.h>

using namespace dsa;

TEST(VariantTest, MsgpackDecoding) {
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);

  {
    msgpack_packer pk;

    /* serialize values into the buffer using msgpack_sbuffer_write callback function. */
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);
  }

  Variant v = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(sbuf.data), sbuf.size);

  EXPECT_TRUE(v.is_array());

  auto vec = v.get_array();

  EXPECT_EQ(vec.size(), 3);

  EXPECT_TRUE(vec[0].is_int());
  EXPECT_EQ(vec[0].get_int(), 1);

  EXPECT_TRUE(vec[1].is_bool());
  EXPECT_EQ(vec[1].get_bool(), true);

  EXPECT_TRUE(vec[2].is_string());
  EXPECT_EQ(vec[2].get_string(), "example");


  msgpack_sbuffer_destroy(&sbuf);
}

