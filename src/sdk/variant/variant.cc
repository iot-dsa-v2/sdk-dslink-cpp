#include "dsa_common.h"

#include "variant.h"

namespace dsa {
const size_t MAX_SIZE_UNSHARED = 256;



VarMap::VarMap(
    std::initializer_list<std::map<string_, Var>::value_type> init)
    : std::map<string_, Var>(init) {}

VarArray::VarArray(std::initializer_list<Var> init) : std::vector<Var>(init) {}

Var::Var(const int64_t v) : BaseVariant(v) {}
Var::Var(const int32_t v) : BaseVariant(static_cast<int64_t>(v)) {}
Var::Var(const uint64_t v) : BaseVariant(static_cast<int64_t>(v)) {}

Var::Var(const double v) : BaseVariant(v) {}

Var::Var(const bool v) : BaseVariant(v) {}

Var::Var(const char* v) {
  size_t size = strlen(v);
  if (size < MAX_SIZE_UNSHARED) {
    *this = string_(v, size);
  } else {
    *this = ref_<const RefCountString>(new RefCountString(v, size));
  }
}

Var::Var(const char* v, size_t size) {
  if (size < MAX_SIZE_UNSHARED) {
    *this = string_(v, size);
  } else {
    *this = ref_<const RefCountString>(new RefCountString(v, size));
  }
}
Var::Var(const string_& v) : BaseVariant(v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = v;
  } else {
    *this = ref_<const RefCountString>(new RefCountString(v));
  }
}
Var::Var(const string_&& v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = std::move(v);
  } else {
    *this = ref_<const RefCountString>(new RefCountString(std::move(v)));
  }
}

Var::Var(const uint8_t* v, size_t size) {
  if (size < MAX_SIZE_UNSHARED) {
    *this = std::vector<uint8_t>(v, v + size);
  } else {
    *this = BytesRef(new RefCountBytes(v, v + size));
  }
}
Var::Var(const std::vector<uint8_t>& v) : BaseVariant(v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = v;
  } else {
    *this = BytesRef(new RefCountBytes(v));
  }
}
Var::Var(const std::vector<uint8_t>&& v) {
  if (v.size() < MAX_SIZE_UNSHARED) {
    *this = std::move(v);
  } else {
    *this = BytesRef(new RefCountBytes(std::move(v)));
  }
}

Var::Var(RefCountString* p) : BaseVariant(ref_<const RefCountString>(p)) {}

Var::Var(RefCountBytes* p) : BaseVariant(BytesRef(p)) {}

Var::Var() : BaseVariant(boost::blank()) {}
Var::Var(VarMap* p) : BaseVariant(ref_<VarMap>(p)) {}
Var::Var(VarArray* p) : BaseVariant(ref_<VarArray>(p)) {}

Var::Var(std::initializer_list<VarMap::value_type> init)
    : BaseVariant(ref_<VarMap>(new VarMap(init))) {}
Var::Var(std::initializer_list<Var> init)
    : BaseVariant(ref_<VarArray>(new VarArray(init))) {}

Var Var::new_map() { return Var(new VarMap()); }
Var Var::new_array() { return Var(new VarArray()); }

int Var::get_type() {
  switch (which()) {
    case Var::SHARED_STRING:
      return Var::STRING;
    case Var::SHARED_BINARY:
      return Var::BINARY;
    default:
      return which();
  }
}
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
  const string_* str_value;
  switch (which()) {
    case DOUBLE:
      return boost::get<double>(*this);
    case INT:
      return boost::get<int64_t>(*this);
    case BOOL:
      return boost::get<bool>(*this) ? 1.0 : 0.0;
    case STRING:
      str_value = &boost::get<string_>(*this);
      break;
    case SHARED_STRING:
      str_value = boost::get<ref_<const RefCountString>>(*this).get();
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
  const string_* str_value;
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
      str_value = &boost::get<string_>(*this);
      break;
    case SHARED_STRING:
      str_value = boost::get<ref_<const RefCountString>>(*this).get();
      break;
    default:
      return defaultout;
  }
  if (*str_value == "true" || *str_value == "True" || *str_value == "TRUE") {
    return true;
  }
  return false;
}
const string_ Var::to_string(const string_& defaultout) const {
  switch (which()) {
    case BOOL:
      return std::to_string(boost::get<bool>(*this));
    case DOUBLE:
      return std::to_string(boost::get<double>(*this));
    case INT:
      return std::to_string(boost::get<int64_t>(*this));
    case STRING:
      return boost::get<string_>(*this);
    case SHARED_STRING:
      return *boost::get<ref_<const RefCountString>>(*this);
    default:
      return defaultout;
  }
}

Var& Var::operator[](const string_& name) {
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
    case SHARED_STRING: {
      return Var(*boost::get<ref_<const RefCountString>>(*this));
    }
    case SHARED_BINARY: {
      return Var(*boost::get<BytesRef>(*this));
    }
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


#define ITERATIVE_STD_EQUAL     if(a.size()  != b.size() ) return false; return std::equal(a.begin(),  a.end(),  b.begin());
#define ITERATIVE_STD_EQUAL_PTR if(a->size() != b->size()) return false; return std::equal(a->begin(), a->end(), b->begin());

struct VAR_equality_visitor : public boost::static_visitor<bool> {
    // Basic types
    bool operator()(const boost::blank& a, const boost::blank& b) const { return true; }
    bool operator()(const short& a, const short& b) const { return a==b; }
    bool operator()(const int& a, const int& b) const { return a==b; }
    bool operator()(const long& a, const long& b) const { return a==b; }
    bool operator()(const double& a, const double& b) const { return a==b; }
    bool operator()(const bool& a, const bool& b) const { return a==b; }

    bool operator()(const ref_<VarArray>& a, const ref_<VarArray>& b) const { return a.get() == b.get(); }
    bool operator()(const ref_<VarMap>& a, const ref_<VarMap>& b) const { return a.get() == b.get(); }

    bool operator()(const std::string& a, const std::string& b) const { ITERATIVE_STD_EQUAL }
    bool operator()(const ref_<const RefCountString>& a, const ref_<const RefCountString>& b) const { ITERATIVE_STD_EQUAL_PTR }
    bool operator()(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) const { ITERATIVE_STD_EQUAL }
    bool operator()(const BytesRef& a, const BytesRef& b) const { ITERATIVE_STD_EQUAL_PTR }

    // Different type checks!
    bool operator()(const ref_<const RefCountString>& a, const BytesRef& b) const { ITERATIVE_STD_EQUAL_PTR }
    bool operator()(const BytesRef& a, const ref_<const RefCountString>& b) const { ITERATIVE_STD_EQUAL_PTR }

    bool operator()(const std::vector<uint8_t>& a, const std::string& b) const { ITERATIVE_STD_EQUAL }
    bool operator()(const std::string& a, const std::vector<uint8_t>& b) const { ITERATIVE_STD_EQUAL }

    // Default operator
    template <typename T1, typename T2>
    bool operator()(const T1& a, const T2& b) const { return false; }
};

struct VAR_equality_visitor var_equality_visitor;
bool operator==(const Var& lhs, const Var& rhs)
{
    return boost::apply_visitor(var_equality_visitor, lhs, rhs);
}



}  // namespace dsa
