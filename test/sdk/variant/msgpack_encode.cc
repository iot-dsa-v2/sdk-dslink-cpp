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
}


