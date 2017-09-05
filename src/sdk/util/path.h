#ifndef DSA_UTIL_PATH_H
#define DSA_UTIL_PATH_H

#include <vector>
#include "enable_intrusive.h"
#include "exception.h"

namespace dsa {

class Path : public EnableRef<Path> {
 public:
  enum Type : uint8_t { INVALID, NODE, CONFIG, ATTRIBUTE };

  std::string full;
  std::vector<std::string> names;
  Type type;

  explicit Path(const std::string &path);
};

class PathRange : public std::vector<std::string> {
 private:
  const ref_<const Path> _path;
  std::vector<std::string>::const_iterator _current;

  PathRange(const ref_<const Path> &path,
            std::vector<std::string>::const_iterator &&it);

 public:
  explicit PathRange(const std::string &path);

  bool invalid() { return _path->type == Path::INVALID; }

  const std::string &current() const { return *_current; }

  bool is_last() const { return _current + 1 != _path->names.end(); }

  const PathRange next() const { return PathRange(_path, _current + 1); }
};
}

#endif  // DSA_UTIL_PATH_H
