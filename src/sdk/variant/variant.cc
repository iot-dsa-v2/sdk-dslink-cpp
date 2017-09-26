#include "dsa_common.h"

#include "variant.h"

namespace dsa {
const size_t MAX_SIZE_UNSHARED = 256;

VariantMap::VariantMap(
    std::initializer_list<std::map<std::string, Variant>::value_type> init)
    : std::map<std::string, Variant>(init) {}

VariantArray::VariantArray(std::initializer_list<Variant> init)
    : std::vector<Variant>(init) {}

Variant::Variant(const int64_t v) : BaseVariant(v) {}
Variant::Variant(const int32_t v) : BaseVariant(static_cast<int64_t>(v)) {}
Variant::Variant(const uint64_t v) : BaseVariant(static_cast<int64_t>(v)) {}

Variant::Variant(const double v) : BaseVariant(v) {}

Variant::Variant(const bool v) : BaseVariant(v) {}

Variant::Variant(const char* v) {
  size_t size = strlen(v);
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::string(v, size);
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(v, size));
  }
}

Variant::Variant(const char* v, size_t size) {
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::string(v, size);
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(v, size));
  }
}
Variant::Variant(const std::string& v) : BaseVariant(v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = v;
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(v));
  }
}
Variant::Variant(const std::string&& v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = std::move(v);
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(std::move(v)));
  }
}

Variant::Variant(const uint8_t* v, size_t size) {
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::vector<uint8_t>(v, v + size);
  } else {
    *this = ref_<IntrusiveBytes>(new IntrusiveBytes(v, v + size));
  }
}
Variant::Variant(const std::vector<uint8_t>& v) : BaseVariant(v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = v;
  } else {
    *this = ref_<IntrusiveBytes>(new IntrusiveBytes(v));
  }
}
Variant::Variant(const std::vector<uint8_t>&& v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = std::move(v);
  } else {
    *this = ref_<IntrusiveBytes>(new IntrusiveBytes(std::move(v)));
  }
}

Variant::Variant(IntrusiveString* p)
    : BaseVariant(ref_<IntrusiveString>(p)) {}

Variant::Variant(IntrusiveBytes* p)
    : BaseVariant(ref_<IntrusiveBytes>(p)) {}

Variant::Variant() : BaseVariant(boost::blank()) {}
Variant::Variant(VariantMap* p) : BaseVariant(ref_<VariantMap>(p)) {}
Variant::Variant(VariantArray* p)
    : BaseVariant(ref_<VariantArray>(p)) {}

Variant::Variant(std::initializer_list<VariantMap::value_type> init)
    : BaseVariant(ref_<VariantMap>(new VariantMap(init))) {}
Variant::Variant(std::initializer_list<Variant> init)
    : BaseVariant(ref_<VariantArray>(new VariantArray(init))) {}

Variant Variant::new_map() { return Variant(new VariantMap()); }
Variant Variant::new_array() { return Variant(new VariantArray()); }

Variant Variant::copy() const {
  switch (which()) {
    case MAP: {
      auto new_map = new VariantMap();
      VariantMap& map = get_map();

      for (auto& it : map) {
        (*new_map)[it.first] = Variant(it.second);
      }
      return Variant(new_map);
    }
    case ARRAY: {
      auto new_array = new VariantArray();
      VariantArray& array = get_array();
      new_array->reserve(array.size());

      for (auto& it : array) {
        new_array->push_back(Variant(it));
      }
      return Variant(new_array);
    }
    default:
      return Variant(*this);
  }
}

Variant Variant::deep_copy() const {
  switch (which()) {
    case MAP: {
      auto new_map = new VariantMap();
      VariantMap& map = get_map();

      for (auto& it : map) {
        (*new_map)[it.first] = std::move(it.second.deep_copy());
      }
      return Variant(new_map);
    }
    case ARRAY: {
      auto new_array = new VariantArray();
      VariantArray& array = get_array();
      new_array->reserve(array.size());

      for (auto& it : array) {
        new_array->push_back(std::move(it.deep_copy()));
      }
      return Variant(new_array);
    }
    default:
      return Variant(*this);
  }
}

}  // namespace dsa
