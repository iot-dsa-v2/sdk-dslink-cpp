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

Path::Path(const std::string &path) {
  if (path.empty()) return;
  auto current = path.begin();
  auto end = path.end();

  if (*current == '/') ++current;

  while (current < end) {
    auto next = std::find(current, end, '/');
    std::string name = std::string(current, next);
    bool is_last = (next == end);
    if (invalid_name(name, is_last)) {
      type = INVALID;
      return;
    }
    if (is_last) {
      const char &c = name[0];
      if (c == '$') {
        type = CONFIG;
      } else if (c == '@') {
        type = ATTRIBUTE;
      } else {
        type = NODE;
      }
    }

    names.emplace_back(std::move(name));
    current = next + 1;
  }
}

PathRange::PathRange(const std::string &path)
    : _path(make_ref_<Path>(path)), _current(_path->names.begin()) {}
PathRange::PathRange(const ref_<const Path> &path,
                     std::vector<std::string>::const_iterator &&it)
    : _path(path), _current(it) {}
}
