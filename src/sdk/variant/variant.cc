#include "variant.h"

#include <msgpack.hpp>

namespace dsa {
Variant::Variant(int64_t v) : BaseVariant(v) {}
Variant::Variant(double v) : BaseVariant(v) {}
Variant::Variant(bool v) : BaseVariant(v) {}
Variant::Variant(const std::string& v)
    : BaseVariant(std::make_shared<std::string>(v)) {}
Variant::Variant(const char* v)
    : BaseVariant(std::make_shared<std::string>(v)) {}

Variant::Variant(const std::shared_ptr<std::map<std::string, Variant>>& v)
    : BaseVariant(v) {}
Variant::Variant(const std::shared_ptr<std::vector<Variant>>& v)
    : BaseVariant(v) {}
Variant::Variant(const BufferPtr& v) : BaseVariant(v) {}
Variant::Variant(const boost::blank& v) : BaseVariant(v) {}

Variant Variant::new_map() {
  return Variant(std::make_shared<std::map<std::string, Variant>>());
}
Variant Variant::new_array() {
  return Variant(std::make_shared<std::vector<Variant>>());
}
Variant Variant::null() { return Variant(boost::blank()); }
}  // namespace dsa
