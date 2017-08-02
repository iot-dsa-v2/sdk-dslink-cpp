#include "dsa_common.h"

#include "variant.h"

#include <msgpack.h>

namespace dsa {

struct MsgpackMemPool {
  msgpack_zone zone;

  MsgpackMemPool() { msgpack_zone_init(&zone, 2048); }
  ~MsgpackMemPool() { msgpack_zone_destroy(&zone); }
};

struct MsgpackSbuffer : public msgpack_sbuffer {
  MsgpackSbuffer() { msgpack_sbuffer_init(this); }
  ~MsgpackSbuffer() { msgpack_sbuffer_destroy(this); }
};

Variant *Variant::to_variant(const msgpack_object &obj) {
  switch (obj.type) {
    case MSGPACK_OBJECT_MAP: {
      auto map = new VariantMap();

      struct msgpack_object_kv *p = obj.via.map.ptr;
      for (size_t i = 0; i < obj.via.map.size; ++i, ++p) {
        // ignore the key if not string
        if (p->key.type == MSGPACK_OBJECT_STR) {
          (*map)[std::string(p->key.via.str.ptr, p->key.via.str.size)] =
              std::unique_ptr<Variant>(to_variant(p->val));
        }
      }
      return new Variant(map);
    }
    case MSGPACK_OBJECT_ARRAY: {
      VariantArray *array = new VariantArray();
      array->reserve(obj.via.array.size);

      struct msgpack_object *p = obj.via.array.ptr;
      for (size_t i = 0; i < obj.via.array.size; ++i, ++p) {
        array->push_back(std::unique_ptr<Variant>(to_variant(*p)));
      }
      return new Variant(array);
    }
    case MSGPACK_OBJECT_STR:return new Variant(obj.via.str.ptr, obj.via.str.size);
    case MSGPACK_OBJECT_POSITIVE_INTEGER:return new Variant(static_cast<int64_t>(obj.via.u64));
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:return new Variant(obj.via.i64);
    case MSGPACK_OBJECT_FLOAT64:return new Variant(obj.via.f64);
    case MSGPACK_OBJECT_BOOLEAN:return new Variant(obj.via.boolean);
    case MSGPACK_OBJECT_BIN:
      return new Variant(reinterpret_cast<const uint8_t *>(obj.via.bin.ptr),
                         obj.via.bin.size);
    default:
      // return null
      // ignore extension
      return new Variant();
  }
}

Variant *Variant::from_msgpack(const uint8_t *data, size_t size) {
  MsgpackMemPool mempool;
  msgpack_object obj;
  msgpack_unpack(reinterpret_cast<const char *>(data), size, NULL,
                 &mempool.zone, &obj);
  return to_variant(obj);
}

void msgpack_pack(msgpack_packer *pk, Variant& v) {
  if (v.is_double()) {
    msgpack_pack_double(pk, v.get_double());
  } else if (v.is_int()) {
    msgpack_pack_int(pk, v.get_int());
  } else if (v.is_bool()) {
    v.get_bool() ? msgpack_pack_true(pk) : msgpack_pack_false(pk);
  } else if (v.is_string()) {
    const std::string& str = v.get_string();
    size_t str_length = str.length();
    msgpack_pack_str(pk, str_length);
    msgpack_pack_str_body(pk, str.c_str(), str_length);
  } else if (v.is_binary()) {
    const std::vector<uint8_t> &bin = v.get_binary();
    size_t bin_size = bin.size();

    uint8_t* buf = new uint8_t[bin_size];
    std::copy(bin.begin(), bin.end(), buf);

    msgpack_pack_bin(pk, bin_size);
    msgpack_pack_bin_body(pk, buf, bin_size);

    delete [] buf;
  } else if (v.is_null()) {
    msgpack_pack_nil(pk);
  } else if (v.is_array()) {
    VariantArray& array = v.get_array();
    msgpack_pack_array(pk, array.size());
    for (auto &it : array) {
      msgpack_pack(pk, *it);
    }
  } else if (v.is_map()) {
    VariantMap& map = v.get_map();
    msgpack_pack_map(pk, map.size());
    for (auto &it : map) {
      std::string key = it.first;
      size_t key_size = key.size();
      msgpack_pack_str(pk, key_size);
      msgpack_pack_str_body(pk, key.c_str(), key_size);
      msgpack_pack(pk, *it.second);
    }
  } else {
    // TODO
  }
}

std::vector<uint8_t> *Variant::to_msgpack() {
  MsgpackSbuffer sbuf;
  msgpack_packer pk;

  msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

  msgpack_pack(&pk, *this);

  std::vector<uint8_t> *v = new std::vector<uint8_t>(sbuf.size);
  v->insert(v->begin(), &sbuf.data[0], &sbuf.data[sbuf.size]);

  return v;
}

}  // namespace dsa
