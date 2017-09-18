#ifndef DSA_SDK_DATETIME_H
#define DSA_SDK_DATETIME_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {

class DateTime {
 public:
  static const std::string& get_ts();
};
}
#endif  // DSA_SDK_DATETIME_H
