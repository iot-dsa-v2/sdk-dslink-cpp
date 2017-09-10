#include "dsa_common.h"

#include "path.h"

static bool invalid_name(const std::string &name, bool is_last) {
  if (name.empty()) return true;
  if (!is_last && (name[0] == '@' || name[0] == '$')) {
    // attribute and metadata name can not show up in parent node
    return true;
  }
  if (name[0] == '.' && (name.size() == 1 || name[1] == '.')) {
    // name can not be '.' or start with '..'
    return true;
  }
  int check_escape = 0;
  for (const char &c : name) {  // invalid characters
    if (check_escape > 0) {
      // % must be followed by 2 upper case hex bytes
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ||
          (c >= 'a' && c <= 'f')) {
        check_escape--;
        continue;
      }
      return true;  // invalid hex
    }

    // invalid characters
    if (c < ' ' || c == '\\' || c == '\'' || c == '\"' || c == '/' ||
        c == '?' || c == '*' || c == '|') {
      return true;
    }
    if (c == '%') {
      check_escape = 2;
    }
  }
  return check_escape != 0;
}

namespace dsa {

PathData::PathData(const std::string &path) : str(path) {
  if (path.empty()) {
    type = ROOT;
    return;
  }
  auto current = path.begin();
  auto end = path.end();

  while (true) {
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
        type = METADATA;
      } else if (first_char == '@') {
        type = ATTRIBUTE;
      } else {
        type = NODE;
      }
      return;
    }
    current = next + 1;
    if (current == end) {
      type = INVALID;
      return;
    }
  }
}

Path::Path(const std::string &path) : _data(make_ref_<PathData>(path)) {}
Path::Path(const ref_<const PathData> &data, size_t idx)
    : _data(data), _current(idx) {}


const Path Path::copy() {
  return Path(ref_<PathData>(new PathData(*_data)), _current);
}
}
