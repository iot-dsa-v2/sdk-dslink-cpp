#include "dsa_common.h"

#include "variant.h"

#include <msgpack.h>

namespace dsa {

struct MsgpackMemPool {
  msgpack_zone zone;

  MsgpackMemPool() { msgpack_zone_init(&zone, 2048); }
  ~MsgpackMemPool() { msgpack_zone_destroy(&zone); }
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

std::vector<uint8_t> *Variant::to_msgpack() {
  msgpack_sbuffer sbuf;
  msgpack_packer pk;

  msgpack_sbuffer_init(&sbuf);
  msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

  if (is_double()) {
    msgpack_pack_double(&pk, get_double());
  } else if (is_int()) {
    msgpack_pack_int(&pk, get_int());
  } else if (is_bool()) {
  } else if (is_string()) {
  } else if (is_map()) {
  } else if (is_array()) {
  } else if (is_binary()) {
  } else if (is_null()) {
  } else {
    // TODO
  }

  //std::ostringstream oss;
  //oss.write(sbuf.data, sbuf.size);

  std::vector<uint8_t> *v = new std::vector<uint8_t>(sbuf.size);
  v->insert(v->begin(), &sbuf.data[0], &sbuf.data[sbuf.size]);

  msgpack_sbuffer_destroy(&sbuf);

  return v;
}

}  // namespace dsa
