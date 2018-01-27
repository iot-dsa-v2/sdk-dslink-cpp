#ifndef DSA_SDK_DATETIME_H
#define DSA_SDK_DATETIME_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {

class DateTime {
 public:

// return ISO8601 timestamp string, i.e. 2017-09-25T14:46.023-07:00
  static const string_& get_ts();

  static int64_t ms_since_epoch();
};
}
#endif  // DSA_SDK_DATETIME_H
