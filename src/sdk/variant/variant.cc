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
    : BaseVariant(std::shared_ptr<const std::string>(p)) {}

Variant::Variant(const uint8_t* v, size_t size)
    : BaseVariant(std::vector<uint8_t>(v, v + size)) {}
Variant::Variant(const std::vector<uint8_t>& v) : BaseVariant(v) {}
Variant::Variant(const std::vector<uint8_t>* p)
    : BaseVariant(std::shared_ptr<const std::vector<uint8_t>>(p)) {}

Variant::Variant() : BaseVariant(boost::blank()) {}
Variant::Variant(VariantMap* p) : BaseVariant(std::shared_ptr<VariantMap>(p)) {}
Variant::Variant(VariantArray* p)
    : BaseVariant(std::shared_ptr<VariantArray>(p)) {}

Variant* Variant::new_map() { return new Variant(new VariantMap()); }
Variant* Variant::new_array() { return new Variant(new VariantArray()); }

Variant* Variant::copy() const {
  switch (which()) {
    case Map: {
      VariantMap* new_map = new VariantMap();
      VariantMap& map = get_map();

      for (VariantMap::iterator it = map.begin(); it != map.end(); ++it) {
        (*new_map)[it->first] =
            std::unique_ptr<Variant>(new Variant(*(it->second)));
      }
      return new Variant(new_map);
    }
    case Array: {
      VariantArray* new_array = new VariantArray();
      VariantArray& array = get_array();
      new_array->reserve(array.size());

      for (VariantArray::iterator it = array.begin(); it != array.end(); ++it) {
        new_array->push_back(std::unique_ptr<Variant>(new Variant(*(it->get()))));
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
      VariantMap* new_map = new VariantMap();
      VariantMap& map = get_map();

      for (VariantMap::iterator it = map.begin(); it != map.end(); ++it) {
        (*new_map)[it->first] =
            std::unique_ptr<Variant>(it->second->deep_copy());
      }
      return new Variant(new_map);
    }
    case Array: {
      VariantArray* new_array = new VariantArray();
      VariantArray& array = get_array();
      new_array->reserve(array.size());

      for (VariantArray::iterator it = array.begin(); it != array.end(); ++it) {
        new_array->push_back(std::unique_ptr<Variant>(it->get()->deep_copy()));
      }
      return new Variant(new_array);
    }
    default:
      return new Variant(*this);
  }
}

}  // namespace dsa
