#include "dsa_common.h"

#include "path.h"

static bool invalid_name(const std::string &name, bool is_last) {
  if (name.empty()) return true;
  if (!is_last && (name[0] == '@' || name[0] == '$')) {
    // attribute and config name can not show up in parent node
    return true;
  }
  if (name[0] == '.' && (name.size() == 1 || name[1] == '.')) {
    // name can not be '.' or start with '..'
    return true;
  }
  for (const char &c : name) {  // invalid characters
    if (c < ' ' || c == '\\' || c == '\'' || c == '\"' || c == '/' ||
        c == '%' || c == '?' || c == '*' || c == ':' || c == '>' || c == '<') {
      return true;
    }
  }
  return false;
}

namespace dsa {

PathData::PathData(const std::string &path) : str(path) {
  if (path.empty()) {
    type = ROOT;
    return;
  }
  auto current = path.begin();
  auto end = path.end();

  if (*current == '/') {
    ++current;
    if (current == end) {
      type = ROOT;
      return;
    }
  }

  while (current < end) {
    auto next = std::find(current, end, '/');
    std::string name = std::string(current, next);
    bool is_last = (next == end);
    if (invalid_name(name, is_last)) {
      type = INVALID;
      return;
    }
    const char first_char = name[0];
    names.emplace_back(std::move(name));
    if (is_last) {
      if (first_char == '$') {
        type = CONFIG;
      } else if (first_char == '@') {
        type = ATTRIBUTE;
      } else {
        type = NODE;
      }
      return;
    }
    current = next + 1;
  }
}

Path::Path(const std::string &path)
    : data(make_ref_<PathData>(path)), _current(0) {}
Path::Path(const ref_<const PathData> &data, size_t idx)
    : data(data), _current(idx) {}

const Path Path::copy() {
  return Path(ref_<PathData>(new PathData(*data)), _current);
}
}
