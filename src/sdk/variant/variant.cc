#include "dsa_common.h"

#include "variant.h"

namespace dsa {
Variant::Variant(int64_t v) : BaseVariant(v) {}
Variant::Variant(int32_t v) : BaseVariant(static_cast<int64_t>(v)) {}
Variant::Variant(int16_t v) : BaseVariant(static_cast<int64_t>(v)) {}

Variant::Variant(double v) : BaseVariant(v) {}

Variant::Variant(bool v) : BaseVariant(v) {}

Variant::Variant(const std::string& v)
    : BaseVariant(std::make_shared<std::string>(v)) {}
Variant::Variant(const char* v)
    : BaseVariant(std::make_shared<std::string>(v)) {}
Variant::Variant(const char* v, size_t size)
    : BaseVariant(std::make_shared<std::string>(v, size)) {}

Variant::Variant(const std::vector<uint8_t>& v)
    : BaseVariant(std::make_shared<const std::vector<uint8_t>>(v)) {}
Variant::Variant(const uint8_t* data, size_t size)
    : BaseVariant(
          std::make_shared<const std::vector<uint8_t>>(data, data + size)) {}

Variant::Variant() : BaseVariant(boost::blank()) {}
Variant::Variant(const std::shared_ptr<std::map<std::string, Variant>>& v)
    : BaseVariant(v) {}
Variant::Variant(const std::shared_ptr<std::vector<Variant>>& v)
    : BaseVariant(v) {}

Variant Variant::new_map() {
  return Variant(std::make_shared<std::map<std::string, Variant>>());
}
Variant Variant::new_array() {
  return Variant(std::make_shared<std::vector<Variant>>());
}

Variant Variant::copy() const {
  switch (which()) {
    case Map:
      return Variant(
          std::make_shared<std::map<std::string, Variant>>(get_map()));
    case Array:
      return Variant(std::make_shared<std::vector<Variant>>(get_array()));
    default:
      return Variant(*this);
  }
}

}  // namespace dsa
