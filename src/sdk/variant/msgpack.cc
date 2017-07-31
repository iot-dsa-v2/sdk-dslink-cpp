#include "dsa_common.h"

#include "variant.h"

#include <msgpack.h>

namespace dsa {

static struct MsgpackMemPool {
  msgpack_zone zone;

  MsgpackMemPool() { msgpack_zone_init(&zone, 2048); }
  ~MsgpackMemPool() { msgpack_zone_destroy(&zone); }
};

static Variant obj_to_variant(const msgpack_object &obj) {
  switch (obj.type) {
    case MSGPACK_OBJECT_MAP: {
      std::shared_ptr<std::map<std::string, Variant>> ptr =
          std::make_shared<std::map<std::string, Variant>>();
      std::map<std::string, Variant> &map = *ptr;

      struct msgpack_object_kv *p = obj.via.map.ptr;
      for (size_t i = 0; i < obj.via.map.size; ++i, ++p) {
        // ignore the key if not string
        if (p->key.type == MSGPACK_OBJECT_STR) {
          map[std::string(p->key.via.str.ptr, p->key.via.str.size)] =
              obj_to_variant(p->val);
        }
      }
      return Variant(ptr);
    }
    case MSGPACK_OBJECT_ARRAY: {
      std::shared_ptr<std::vector<Variant>> ptr =
          std::make_shared<std::vector<Variant>>();
      std::vector<Variant> &array = *ptr;
      array.reserve(obj.via.array.size);

      struct msgpack_object *p = obj.via.array.ptr;
      for (size_t i = 0; i < obj.via.array.size; ++i, ++p) {
        array.push_back(obj_to_variant(*p));
      }
      return Variant(ptr);
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
  return obj_to_variant(obj);
}

std::vector<const std::vector<uint8_t> *> Variant::to_msgpack() {
  return std::vector<const std::vector<uint8_t> *>();
}

}  // namespace dsa
