#ifndef DSA_UTIL_PATH_H
#define DSA_UTIL_PATH_H

#include <vector>
#include "enable_intrusive.h"
#include "exception.h"

namespace dsa {

class PathData : public EnableRef<PathData> {
 public:
  enum Type : uint8_t { INVALID, ROOT, NODE, CONFIG, ATTRIBUTE };

  std::string str;
  std::vector<std::string> names;
  Type type;

  explicit PathData(const std::string &path);
};

class Path {
 private:
  size_t _current;

  Path(const ref_<const PathData> &data, size_t idx);

 public:
  const ref_<const PathData> data;

  explicit Path(const std::string &path);

  bool is_invalid() const { return data->type == PathData::INVALID; }

  const std::string &current() const { return data->names[_current]; }

  bool is_last() const { return _current + 1 == data->names.size(); }

  const Path next() const { return Path(data, _current + 1); }

  // deep copy the path to share with other thread
  const Path copy();
};
}

#endif  // DSA_UTIL_PATH_H
