#include "dsa_common.h"

#include "date_time.h"

#include <chrono>
#include <iomanip>

#if _MSC_VER
#define localtime_r(a, b) localtime_s(b, a)
#endif

namespace dsa {

thread_local std::chrono::milliseconds _last_ms{0};
thread_local string_ _last_ts;

static void update_ts(std::chrono::system_clock::time_point now) {
  std::time_t t = std::chrono::system_clock::to_time_t(now);

  tm localt;
  localtime_r(&t, &localt);

  char buf[32];
  size_t str_size = std::strftime(buf, sizeof(buf), "%FT%T.000%z0", &localt);
  string_ str(buf, str_size);

  // set milli second;
  auto ms = _last_ms.count() % 1000;
  str[20] = static_cast<char>((ms / 100) + '0');
  str[21] = static_cast<char>(((ms % 100) / 10) + '0');
  str[22] = static_cast<char>((ms % 10) + '0');

  // fix format of timezone;
  str[28] = str[27];
  str[27] = str[26];
  str[26] = ':';

  _last_ts = std::move(str);
}

const string_& DateTime::get_ts() {
  // only generate the string when time changed
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch());
  if (ms != _last_ms) {
    _last_ms = ms;
    update_ts(now);
  }
  return _last_ts;
}
int64_t DateTime::time_since_epoch() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             now.time_since_epoch())
      .count();
}
}
