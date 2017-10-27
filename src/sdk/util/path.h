#ifndef DSA_UTIL_PATH_H
#define DSA_UTIL_PATH_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>
#include "enable_ref.h"
#include "exception.h"

namespace dsa {

class Path;

class PathData : public EnableRef<PathData> {
  friend class Path;

 public:
  string_ str;
  std::vector<string_> names;
  bool is_root = false;
  bool invalid = false;

  explicit PathData(const string_ &path);

  explicit PathData(std::vector<string_> &&names);
};

class Path {
 private:
  size_t _current = 0;
  ref_<const PathData> _data;
  Path(const ref_<const PathData> &data, size_t idx);

 public:
  Path() = default;
  explicit Path(const string_ &path);

  const ref_<const PathData> &data() { return _data; }

  bool is_invalid() const { return _data == nullptr || _data->invalid; }
  bool is_root() const { return _data->is_root; }

  const size_t &current_pos() const { return _current; }
  const string_ &current_name() const { return _data->names[_current]; }

  const string_ &last_name() const {
    return _data->names[_data->names.size() - 1];
  }

  const string_ &full_str() const { return _data->str; }

  const string_ remain_str() const;

  // last part of the path
  bool is_last() const { return _current + 1 == _data->names.size(); }

  bool is_end() const { return _current == _data->names.size(); }

  const Path next() const { return Path(_data, _current + 1); }
  const Path previous() const { return Path(_data, _current - 1); }
  const Path move_pos(size_t pos) const { return Path(_data, pos); }

  const Path get_child_path(const string_ &name);

  const Path get_parent_path();

  // deep copy the path to share with other thread
  const Path copy();
};
}

#endif  // DSA_UTIL_PATH_H
