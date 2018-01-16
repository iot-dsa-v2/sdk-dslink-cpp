#include "dsa/variant.h"
#include <gtest/gtest.h>

#include <msgpack.h>

using namespace dsa;

TEST(VariantTest, MsgpackEncodingDouble) {
  Var v(1.23);

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_double());
  EXPECT_EQ(1.23, v_dash.get_double());
}

TEST(VariantTest, MsgpackEncodingInt) {
  Var v(123);

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_int());
  EXPECT_EQ(123, v_dash.get_int());
}

TEST(VariantTest, MsgpackEncodingBool) {
  {
    Var v(true);

    std::vector<uint8_t> encoded_msg = v.to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg.size();

    Var v_dash = Var::from_msgpack(
        reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash.is_bool());
    EXPECT_TRUE(v_dash.get_bool());
  }

  {
    Var v(false);

    std::vector<uint8_t> encoded_msg = v.to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg.size();

    Var v_dash = Var::from_msgpack(
        reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash.is_bool());
    EXPECT_FALSE(v_dash.get_bool());
  }
}

TEST(VariantTest, MsgpackEncodingString) {
  Var v("hello");

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_string());
  EXPECT_EQ("hello", v_dash.get_string());
}

TEST(VariantTest, MsgpackEncodingNull) {
  Var v;

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_null());
}

TEST(VariantTest, MsgpackEncodingBinary) {
  Var v;
  {
    std::vector<uint8_t> vec = {0, 1, 2};
    Var v0(vec);
    v = v0;
  }

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_binary());

  const std::vector<uint8_t> &vec = v_dash.get_binary();
  EXPECT_EQ(0, vec[0]);
  EXPECT_EQ(1, vec[1]);
  EXPECT_EQ(2, vec[2]);
}

TEST(VariantTest, MsgpackEncodingArray) {
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);

  {
    msgpack_packer pk;

    /* serialize values into the buffer using msgpack_sbuffer_write callback
     * function. */
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);
  }

  msgpack_zone mempool;
  msgpack_zone_init(&mempool, 2048);

  msgpack_object obj;
  msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &obj);

  class VariantExt : public Var {
   public:
    static Var to_variant_ext(const msgpack_object &obj) {
      return to_variant(obj);
    }
  };

  Var v = VariantExt::to_variant_ext(obj);

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_array());

  VarArray &vec = v_dash.get_array();

  EXPECT_EQ(3, vec.size());

  EXPECT_TRUE(vec[0].is_int());
  EXPECT_EQ(1, vec[0].get_int());

  EXPECT_TRUE(vec[1].is_bool());
  EXPECT_TRUE(vec[1].get_bool());

  EXPECT_TRUE(vec[2].is_string());
  EXPECT_EQ("example", vec[2].get_string());

  msgpack_zone_destroy(&mempool);
  msgpack_sbuffer_destroy(&sbuf);
}

TEST(VariantTest, MsgpackEncodingNestedArray) {
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);

  {
    msgpack_packer pk;

    /* serialize values into the buffer using msgpack_sbuffer_write callback
     * function. */
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 4);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);
    msgpack_pack_map(&pk, 2);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "dsid", 4);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "path", 4);
    msgpack_pack_str(&pk, 10);
    msgpack_pack_str_body(&pk, "/path/name", 10);
  }

  msgpack_zone mempool;
  msgpack_zone_init(&mempool, 2048);

  msgpack_object obj;
  msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &obj);

  class VariantExt : public Var {
   public:
    static Var to_variant_ext(const msgpack_object &obj) {
      return to_variant(obj);
    }
  };

  Var v = VariantExt::to_variant_ext(obj);

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_array());

  VarArray &vec = v_dash.get_array();

  EXPECT_EQ(4, vec.size());

  EXPECT_TRUE(vec[0].is_int());
  EXPECT_EQ(1, vec[0].get_int());

  EXPECT_TRUE(vec[1].is_bool());
  EXPECT_TRUE(vec[1].get_bool());

  EXPECT_TRUE(vec[2].is_string());
  EXPECT_EQ("example", vec[2].get_string());

  EXPECT_TRUE(vec[3].is_map());
  VarMap &map = vec[3].get_map();

  EXPECT_TRUE(map["dsid"].is_int());
  EXPECT_EQ(1, map["dsid"].get_int());

  EXPECT_TRUE(map["path"].is_string());
  EXPECT_EQ("/path/name", map["path"].get_string());

  msgpack_zone_destroy(&mempool);
  msgpack_sbuffer_destroy(&sbuf);
}

TEST(VariantTest, MsgpackEncodingMap) {
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);

  {
    msgpack_packer pk;

    /* serialize values into the buffer using msgpack_sbuffer_write callback
     * function. */
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_map(&pk, 2);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "dsid", 4);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "path", 4);
    msgpack_pack_str(&pk, 10);
    msgpack_pack_str_body(&pk, "/path/name", 10);
  }

  Var v = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(sbuf.data), sbuf.size);

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_map());

  VarMap &map = v_dash.get_map();

  EXPECT_EQ(2, map.size());

  EXPECT_TRUE(map["dsid"].is_int());
  EXPECT_EQ(1, map["dsid"].get_int());

  EXPECT_TRUE(map["path"].is_string());
  EXPECT_EQ("/path/name", map["path"].get_string());

  msgpack_sbuffer_destroy(&sbuf);
}

TEST(VariantTest, MsgpackEncodingNestedMap) {
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);

  {
    msgpack_packer pk;

    /* serialize values into the buffer using msgpack_sbuffer_write callback
     * function. */
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_map(&pk, 3);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "dsid", 4);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "path", 4);
    msgpack_pack_str(&pk, 10);
    msgpack_pack_str_body(&pk, "/path/name", 10);
    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "value", 5);
    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);
  }

  Var v = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(sbuf.data), sbuf.size);

  std::vector<uint8_t> encoded_msg = v.to_msgpack();
  uint8_t encoded_buf[1024];
  std::copy(encoded_msg.begin(), encoded_msg.end(), encoded_buf);
  size_t encoded_buf_size = encoded_msg.size();

  Var v_dash = Var::from_msgpack(
      reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

  EXPECT_TRUE(v_dash.is_map());

  VarMap &map = v_dash.get_map();

  EXPECT_EQ(3, map.size());

  EXPECT_TRUE(map["dsid"].is_int());
  EXPECT_EQ(1, map["dsid"].get_int());

  EXPECT_TRUE(map["path"].is_string());
  EXPECT_EQ("/path/name", map["path"].get_string());

  EXPECT_TRUE(map["value"].is_array());
  VarArray &vec = map["value"].get_array();

  EXPECT_TRUE(vec[0].is_int());
  EXPECT_EQ(1, vec[0].get_int());

  EXPECT_TRUE(vec[1].is_bool());
  EXPECT_TRUE(vec[1].get_bool());

  EXPECT_TRUE(vec[2].is_string());
  EXPECT_EQ("example", vec[2].get_string());

  msgpack_sbuffer_destroy(&sbuf);
}
