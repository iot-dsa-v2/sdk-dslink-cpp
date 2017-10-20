#include "dsa_common.h"

#include "variant.h"

namespace dsa {
const size_t MAX_SIZE_UNSHARED = 256;

VarMap::VarMap(
    std::initializer_list<std::map<std::string, Var>::value_type> init)
    : std::map<std::string, Var>(init) {}

VarArray::VarArray(std::initializer_list<Var> init)
    : std::vector<Var>(init) {}

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
