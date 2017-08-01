#include "dsa/variant.h"
#include "gtest/gtest.h"

#include <msgpack.h>

using namespace dsa;
typedef std::unique_ptr<Variant> VariantPtr;

TEST(VariantTest, MsgpackEncoding) {

  {
    VariantPtr v = VariantPtr(Variant::create(1.23));

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_double());
    EXPECT_EQ(1.23, v_dash->get_double());
  }
  {
    VariantPtr v = VariantPtr(Variant::create(123));

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_int());
    EXPECT_EQ(123, v_dash->get_int());
  }
  {
    VariantPtr v = VariantPtr(Variant::create(true));

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_bool());
    EXPECT_EQ(true, v_dash->get_bool());
  }
  {
    VariantPtr v = VariantPtr(Variant::create(false));

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_bool());
    EXPECT_EQ(false, v_dash->get_bool());
  }
  {
    VariantPtr v = VariantPtr(Variant::create("hello"));

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_string());
    EXPECT_EQ("hello", v_dash->get_string());
  }
  {
    VariantPtr v = VariantPtr(new Variant);

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_null());
  }
  {
    VariantPtr v = VariantPtr(new Variant);
    {
      std::vector<uint8_t> vec = {0, 1, 2};
      VariantPtr v0 = VariantPtr(Variant::create(vec));
      *v = *v0;
    }

    std::vector<uint8_t> *encoded_msg = v->to_msgpack();
    uint8_t encoded_buf[1024];
    std::copy(encoded_msg->begin(), encoded_msg->end(), encoded_buf);
    size_t encoded_buf_size = encoded_msg->size();

    Variant *v_dash = Variant::from_msgpack(reinterpret_cast<const uint8_t *>(encoded_buf), encoded_buf_size);

    EXPECT_TRUE(v_dash->is_binary());

    const std::vector<uint8_t>& vec = v_dash->get_binary();
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 1);
    EXPECT_EQ(vec[2], 2);
  }
  {
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

    msgpack_zone mempool;
    msgpack_zone_init(&mempool, 2048);

    msgpack_object obj;
    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &obj);

    class VariantExt : public Variant {
     public:
      static Variant *to_variant_ext(const msgpack_object &obj) {
        return to_variant(obj);
      }
    };

    Variant *v = VariantExt::to_variant_ext(obj);

    EXPECT_TRUE(v->is_array());

    VariantArray& vec = v->get_array();

    EXPECT_EQ(vec.size(), 3);

    EXPECT_TRUE(vec[0]->is_int());
    EXPECT_EQ(vec[0]->get_int(), 1);

    EXPECT_TRUE(vec[1]->is_bool());
    EXPECT_EQ(vec[1]->get_bool(), true);

    EXPECT_TRUE(vec[2]->is_string());
    EXPECT_EQ(vec[2]->get_string(), "example");

    msgpack_zone_destroy(&mempool);
    msgpack_sbuffer_destroy(&sbuf);
  }
}


