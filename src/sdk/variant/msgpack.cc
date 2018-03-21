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
  ~MsgpackSbuffer() {
    try {
      if (data != nullptr) free(data);
    } catch (std::exception &e) {
    }
  }
};

#ifndef __MINGW32__
// mingw's thread_local is buggy
thread_local MsgpackSbuffer sbuf;
#endif

Var Var::to_variant(const msgpack_object &obj) {
  switch (obj.type) {
    case MSGPACK_OBJECT_MAP: {
      auto map = new VarMap();

      struct msgpack_object_kv *p = obj.via.map.ptr;
      for (size_t i = 0; i < obj.via.map.size; ++i, ++p) {
        // ignore the key if not string
        if (p->key.type == MSGPACK_OBJECT_STR) {
          (*map)[string_(p->key.via.str.ptr, p->key.via.str.size)] =
              to_variant(p->val);
        }
      }
      return Var(map);
    }
    case MSGPACK_OBJECT_ARRAY: {
      auto array = new VarArray();
      array->reserve(obj.via.array.size);

      struct msgpack_object *p = obj.via.array.ptr;
      for (size_t i = 0; i < obj.via.array.size; ++i, ++p) {
        array->push_back(to_variant(*p));
      }
      return Var(array);
    }
    case MSGPACK_OBJECT_STR:
      return Var(obj.via.str.ptr, obj.via.str.size);
    case MSGPACK_OBJECT_POSITIVE_INTEGER:
      return Var(static_cast<int64_t>(obj.via.u64));
    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
      return Var(obj.via.i64);
    case MSGPACK_OBJECT_FLOAT64:
      return Var(obj.via.f64);
    case MSGPACK_OBJECT_BOOLEAN:
      return Var(obj.via.boolean);
    case MSGPACK_OBJECT_BIN:
      return Var(reinterpret_cast<const uint8_t *>(obj.via.bin.ptr),
                 obj.via.bin.size);
    default:
      // return null
      // ignore extension
      return Var();
  }
}

Var Var::from_msgpack(const uint8_t *data, size_t size) {
  if (size == 0) {
    return Var(MessageStatus::BLANK);
  }
  MsgpackMemPool mempool;
  msgpack_object obj;
  auto result = msgpack_unpack(reinterpret_cast<const char *>(data), size, NULL,
                               &mempool.zone, &obj);
  if (result == MSGPACK_UNPACK_SUCCESS ||
      result == MSGPACK_UNPACK_EXTRA_BYTES) {
    return Var(to_variant(obj));
  } else {
    return Var();
  }
}

bool msgpack_pack(msgpack_packer *pk, const Var &v) {
  bool rc = true;

  if (v.is_double()) {
    msgpack_pack_double(pk, v.get_double());
  } else if (v.is_int()) {
    msgpack_pack_int64(pk, v.get_int());
  } else if (v.is_bool()) {
    v.get_bool() ? msgpack_pack_true(pk) : msgpack_pack_false(pk);
  } else if (v.is_string()) {
    const string_ &str = v.get_string();
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
    VarArray &array = v.get_array();
    msgpack_pack_array(pk, array.size());
    for (auto &it : array) {
      msgpack_pack(pk, it);
    }
  } else if (v.is_map()) {
    VarMap &map = v.get_map();
    msgpack_pack_map(pk, map.size());
    for (auto &it : map) {
      string_ key = it.first;
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

std::vector<uint8_t> Var::to_msgpack() const throw(const EncodingError &) {
#ifdef __MINGW32__
  // mingw's thread_local is buggy
  MsgpackSbuffer sbuf;
#else
  sbuf;
#endif
  if (is_status()) {
    // status is not valid value
    return std::vector<uint8_t>();
  }
  if (msgpack_pack(&pk, *this)) {
    size_t sbuf_size = sbuf.size;

    sbuf.size = 0;
    if (sbuf.alloc >= 0x100000) {
      std::vector<uint8_t> v(&sbuf.data[0], &sbuf.data[sbuf_size]);

      free(sbuf.data);
      sbuf.data = nullptr;
      sbuf.alloc = 0;

      return std::move(v);
    }

    return std::vector<uint8_t>(&sbuf.data[0], &sbuf.data[sbuf_size]);
  }

  throw EncodingError("Failed to pack Var to msgpack");
}

Var Var::from_msgpack_pages(std::vector<BytesRef> &pages,
                            const uint8_t *extra_first_page,
                            size_t first_size) {
  size_t len = first_size;
  for (auto &page : pages) {
    len += page->size();
  }
  std::vector<uint8_t> data;
  data.reserve(len);
  if (first_size) {
    data.assign(extra_first_page, extra_first_page + first_size);
  }
  for (auto &page : pages) {
    data.insert(data.end(), page->begin(), page->end());
  }
  return Var::from_msgpack(data.data(), data.size());
}

std::vector<BytesRef> Var::to_msgpack_pages(size_t first_page_size) const
    throw(const EncodingError &) {
  std::vector<uint8_t> data = to_msgpack();

  if (data.size() <= first_page_size) {
    std::vector<BytesRef> result;
    result.emplace_back(make_ref_<RefCountBytes>(std::move(data)));
    return std::move(result);
  } else {
    return split_pages(data);
  }
}
std::vector<BytesRef> Var::split_pages(const std::vector<uint8_t> &data,
                                       size_t first_page_size) {
  std::vector<BytesRef> result;
  size_t remain_size = data.size();
  const uint8_t *pdata = data.data();
  if (first_page_size < MAX_PAGE_BODY_SIZE) {
    // first page might need to save some space for message metadata
    result.emplace_back(
        make_ref_<RefCountBytes>(pdata, pdata + first_page_size));
    remain_size -= first_page_size;
    pdata += first_page_size;
  }
  while (remain_size > MAX_PAGE_BODY_SIZE) {
    result.emplace_back(
        make_ref_<RefCountBytes>(pdata, pdata + MAX_PAGE_BODY_SIZE));
    remain_size -= MAX_PAGE_BODY_SIZE;
    pdata += MAX_PAGE_BODY_SIZE;
  }
  // remain_size won't be 0
  result.emplace_back(make_ref_<RefCountBytes>(pdata, pdata + remain_size));
  return std::move(result);
}

}  // namespace dsa
