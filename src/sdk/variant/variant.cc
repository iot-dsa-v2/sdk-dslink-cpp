#include "dsa_common.h"

#include "variant.h"

namespace dsa {
Variant::Variant(int64_t v) : BaseVariant(v) {}

Variant::Variant(double v) : BaseVariant(v) {}

Variant::Variant(bool v) : BaseVariant(v) {}

Variant::Variant(const char* v, size_t size)
    : BaseVariant(std::string(v, size)) {}
Variant::Variant(const std::string& v) : BaseVariant(v) {}
Variant::Variant(const std::string* p)
  : BaseVariant(intrusive_ptr_<VariantString>(new VariantString(*p))) {}

Variant::Variant(const uint8_t* v, size_t size)
    : BaseVariant(std::vector<uint8_t>(v, v + size)) {}
Variant::Variant(const std::vector<uint8_t>& v) : BaseVariant(v) {}
Variant::Variant(const std::vector<uint8_t>* p)
  : BaseVariant(intrusive_ptr_<VariantBinary>(new VariantBinary(*p))) {}

Variant::Variant() : BaseVariant(boost::blank()) {}
Variant::Variant(VariantMap* p) : BaseVariant(intrusive_ptr_<VariantMap>(p)) {}
Variant::Variant(VariantArray* p) : BaseVariant(intrusive_ptr_<VariantArray>(p)) {}

Variant* Variant::new_map() { return new Variant(new VariantMap()); }
Variant* Variant::new_array() { return new Variant(new VariantArray()); }

Variant* Variant::copy() const {
  switch (which()) {
    case Map: {
      auto new_map = new VariantMap();
      VariantMap& map = get_map();

      for (auto &it : map) {
        (*new_map)[it.first] = Variant(it.second);
      }
      return new Variant(new_map);
    }
    case Array: {
      auto new_array = new VariantArray();
      VariantArray& array = get_array();
      new_array->reserve(array.size());

      for (auto &it : array) {
        new_array->push_back(Variant(it));
      }
      return new Variant(new_array);
    }
    default:
      return new Variant(*this);
  }
}

Variant* Variant::deep_copy() const {
  switch (which()) {
    case Map: {
      auto new_map = new VariantMap();
      VariantMap& map = get_map();

      for (auto &it : map) {
        (*new_map)[it.first] = *(it.second.deep_copy());
      }
      return new Variant(new_map);
    }
    case Array: {
      auto new_array = new VariantArray();
      VariantArray& array = get_array();
      new_array->reserve(array.size());

      for (auto &it : array) {
        new_array->push_back(*(it.deep_copy()));
      }
      return new Variant(new_array);
    }
    default:
      return new Variant(*this);
  }
}

}  // namespace dsa