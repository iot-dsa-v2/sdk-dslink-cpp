#include "dsa_common.h"

#include "variant.h"

namespace dsa {
const size_t MAX_SIZE_UNSHARED = 256;

VarMap::VarMap(
    std::initializer_list<std::map<std::string, Var>::value_type> init)
    : std::map<std::string, Var>(init) {}

VarArray::VarArray(std::initializer_list<Var> init) : std::vector<Var>(init) {}

Var::Var(const int64_t v) : BaseVariant(v) {}
Var::Var(const int32_t v) : BaseVariant(static_cast<int64_t>(v)) {}
Var::Var(const uint64_t v) : BaseVariant(static_cast<int64_t>(v)) {}

Var::Var(const double v) : BaseVariant(v) {}

Var::Var(const bool v) : BaseVariant(v) {}

Var::Var(const char* v) {
  size_t size = strlen(v);
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::string(v, size);
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(v, size));
  }
}

Var::Var(const char* v, size_t size) {
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::string(v, size);
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(v, size));
  }
}
Var::Var(const std::string& v) : BaseVariant(v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = v;
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(v));
  }
}
Var::Var(const std::string&& v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = std::move(v);
  } else {
    *this = ref_<IntrusiveString>(new IntrusiveString(std::move(v)));
  }
}

Var::Var(const uint8_t* v, size_t size) {
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::vector<uint8_t>(v, v + size);
  } else {
    *this = BytesRef(new IntrusiveBytes(v, v + size));
  }
}
Var::Var(const std::vector<uint8_t>& v) : BaseVariant(v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = v;
  } else {
    *this = BytesRef(new IntrusiveBytes(v));
  }
}
Var::Var(const std::vector<uint8_t>&& v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = std::move(v);
  } else {
    *this = BytesRef(new IntrusiveBytes(std::move(v)));
  }
}

Var::Var(IntrusiveString* p) : BaseVariant(ref_<IntrusiveString>(p)) {}

Var::Var(IntrusiveBytes* p) : BaseVariant(BytesRef(p)) {}

Var::Var() : BaseVariant(boost::blank()) {}
Var::Var(VarMap* p) : BaseVariant(ref_<VarMap>(p)) {}
Var::Var(VarArray* p) : BaseVariant(ref_<VarArray>(p)) {}

Var::Var(std::initializer_list<VarMap::value_type> init)
    : BaseVariant(ref_<VarMap>(new VarMap(init))) {}
Var::Var(std::initializer_list<Var> init)
    : BaseVariant(ref_<VarArray>(new VarArray(init))) {}

Var Var::new_map() { return Var(new VarMap()); }
Var Var::new_array() { return Var(new VarArray()); }

Var Var::copy() const {
  switch (which()) {
    case MAP: {
      auto new_map = new VarMap();
      VarMap& map = get_map();

      for (auto& it : map) {
        (*new_map)[it.first] = Var(it.second);
      }
      return Var(new_map);
    }
    case ARRAY: {
      auto new_array = new VarArray();
      VarArray& array = get_array();
      new_array->reserve(array.size());

      for (auto& it : array) {
        new_array->push_back(Var(it));
      }
      return Var(new_array);
    }
    default:
      return Var(*this);
  }
}

double Var::to_double(double defaultout) const {
  const std::string* str_value;
  switch (which()) {
    case DOUBLE:
      return boost::get<double>(*this);
    case INT:
      return boost::get<int64_t>(*this);
    case BOOL:
      return boost::get<bool>(*this) ? 1.0 : 0.0;
    case STRING:
      str_value = &boost::get<std::string>(*this);
      break;
    case SHARED_STRING:
      str_value = boost::get<ref_<IntrusiveString>>(*this).get();
      break;
    default:
      return defaultout;
  }
  try {
    return std::stod(*str_value);
  } catch (std::exception& err) {
    return defaultout;
  }
}
int64_t Var::to_bool(bool defaultout) const {
  const std::string* str_value;
  switch (which()) {
    case BOOL:
      return boost::get<bool>(*this);
    case DOUBLE: {
      double v = boost::get<double>(*this);
      return (v != 0.0 && v == v);  // not 0 or NaN
    }
    case INT:
      return boost::get<int64_t>(*this) != 0;
    case STRING:
      str_value = &boost::get<std::string>(*this);
      break;
    case SHARED_STRING:
      str_value = boost::get<ref_<IntrusiveString>>(*this).get();
      break;
    default:
      return defaultout;
  }
  if (*str_value == "true" || *str_value == "True" || *str_value == "TRUE") {
    return true;
  }
  return false;
}
const std::string& Var::to_string(const std::string& defaultout) const {
  switch (which()) {
    case BOOL:
      return std::to_string(boost::get<bool>(*this));
    case DOUBLE:
      return std::to_string(boost::get<double>(*this));
    case INT:
      return std::to_string(boost::get<int64_t>(*this));
    case STRING:
      return boost::get<std::string>(*this);
    case SHARED_STRING:
      return *boost::get<ref_<IntrusiveString>>(*this);
    default:
      return defaultout;
  }
}

Var& Var::operator[](const std::string& name) {
  if (which() == MAP) {
    return get_map().at(name);
  }
  throw std::out_of_range("Varian is not a map");
}
Var& Var::operator[](size_t index) {
  if (which() == ARRAY) {
    return get_array().at(index);
  }
  throw std::out_of_range("Varian is not an array");
}

Var Var::deep_copy() const {
  switch (which()) {
    case MAP: {
      auto new_map = new VarMap();
      VarMap& map = get_map();

      for (auto& it : map) {
        (*new_map)[it.first] = std::move(it.second.deep_copy());
      }
      return Var(new_map);
    }
    case ARRAY: {
      auto new_array = new VarArray();
      VarArray& array = get_array();
      new_array->reserve(array.size());

      for (auto& it : array) {
        new_array->push_back(std::move(it.deep_copy()));
      }
      return Var(new_array);
    }
    default:
      return Var(*this);
  }
}

}  // namespace dsa
