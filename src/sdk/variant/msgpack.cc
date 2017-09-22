#include "dsa_common.h"

#include "variant.h"

#include <msgpack.h>

namespace dsa {

struct MsgpackMemPool {
  msgpack_zone zone;

  MsgpackMemPool() { msgpack_zone_init(&zone, 2048); }
  ~MsgpackMemPool() { msgpack_zone_destroy(&zone); }
};

thread_local msgpack_packer pk;

struct MsgpackSbuffer : public msgpack_sbuffer {
  MsgpackSbuffer() {
    msgpack_sbuffer_init(this);
    msgpack_packer_init(&pk, this, msgpack_sbuffer_write);
  }
  ~MsgpackSbuffer() { msgpack_sbuffer_destroy(this); }
};

thread_local MsgpackSbuffer sbuf;

Variant Variant::to_variant(const msgpack_object &obj) {
  switch (obj.type) {
    case MSGPACK_OBJECT_MAP: {
      auto map = new VariantMap();

      struct msgpack_object_kv *p = obj.via.map.ptr;
      for (size_t i = 0; i < obj.via.map.size; ++i, ++p) {
        // ignore the key if not string
        if (p->key.type == MSGPACK_OBJECT_STR) {
          (*map)[std::string(p->key.via.str.ptr, p->key.via.str.size)] =
              to_variant(p->val);
        }
      }
      return Variant(map);
    }
    case MSGPACK_OBJECT_ARRAY: {
      auto array = new VariantArray();
      array->reserve(obj.via.array.size);

      struct msgpack_object *p = obj.via.array.ptr;
      for (size_t i = 0; i < obj.via.array.size; ++i, ++p) {
        array->push_back(to_variant(*p));
      }
      return Variant(array);
    }
    case MSGPACK_OBJECT_STR:
      return Variant(obj.via.str.ptr, obj.via.str.size);
    case MSGPACK_OBJECT_POSITIVE_INTEGER:
      return Variant(static_cast<int64_t>(obj.via.u64));
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
      return Variant(obj.via.i64);
    case MSGPACK_OBJECT_FLOAT64:
      return Variant(obj.via.f64);
    case MSGPACK_OBJECT_BOOLEAN:
      return Variant(obj.via.boolean);
    case MSGPACK_OBJECT_BIN:
      return Variant(reinterpret_cast<const uint8_t *>(obj.via.bin.ptr),
                     obj.via.bin.size);
    default:
      // return null
      // ignore extension
      return Variant();
  }
}

Variant Variant::from_msgpack(const uint8_t *data, size_t size) {
  MsgpackMemPool mempool;
  msgpack_object obj;
  msgpack_unpack(reinterpret_cast<const char *>(data), size, NULL,
                 &mempool.zone, &obj);
  return Variant(to_variant(obj));
}

bool msgpack_pack(msgpack_packer *pk, Variant &v) {
  bool rc = true;

  if (v.is_double()) {
    msgpack_pack_double(pk, v.get_double());
  } else if (v.is_int()) {
    msgpack_pack_int(pk, v.get_int());
  } else if (v.is_bool()) {
    v.get_bool() ? msgpack_pack_true(pk) : msgpack_pack_false(pk);
  } else if (v.is_string()) {
    const std::string &str = v.get_string();
    size_t str_length = str.length();
    msgpack_pack_str(pk, str_length);
    msgpack_pack_str_body(pk, str.c_str(), str_length);
  } else if (v.is_binary()) {
    const std::vector<uint8_t> &bin = v.get_binary();
    size_t bin_size = bin.size();

    uint8_t *buf = new uint8_t[bin_size];
    std::copy(bin.begin(), bin.end(), buf);

    msgpack_pack_bin(pk, bin_size);
    msgpack_pack_bin_body(pk, buf, bin_size);

    delete[] buf;
  } else if (v.is_null()) {
    msgpack_pack_nil(pk);
  } else if (v.is_array()) {
    VariantArray &array = v.get_array();
    msgpack_pack_array(pk, array.size());
    for (auto &it : array) {
      msgpack_pack(pk, it);
    }
  } else if (v.is_map()) {
    VariantMap &map = v.get_map();
    msgpack_pack_map(pk, map.size());
    for (auto &it : map) {
      std::string key = it.first;
      size_t key_size = key.size();
      msgpack_pack_str(pk, key_size);
      msgpack_pack_str_body(pk, key.c_str(), key_size);
      msgpack_pack(pk, it.second);
    }
  } else {
    rc = false;
  }

  return rc;
}

std::vector<uint8_t> Variant::to_msgpack() throw(const EncodingError &) {
  sbuf;
  if (msgpack_pack(&pk, *this)) {
    size_t sbuf_size = sbuf.size;

    sbuf.size = 0;
    if (sbuf.alloc >= 0x100000) {
      std::vector<uint8_t> v(&sbuf.data[0], &sbuf.data[sbuf_size]);

      free(sbuf.data);
      sbuf.data = nullptr;
      sbuf.alloc = 0;

      return v;
    }

    return std::vector<uint8_t>(&sbuf.data[0], &sbuf.data[sbuf_size]);
  }

  throw EncodingError("Failed to pack Variant to msgpack");
}

}  // namespace dsa
