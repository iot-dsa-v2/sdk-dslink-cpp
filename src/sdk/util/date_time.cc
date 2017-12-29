#include "dsa_common.h"

#include "date_time.h"

#include <chrono>
#include <iomanip>

#ifdef __MINGW32__
#include <cstring>
#include <timezoneapi.h>
#endif

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
  size_t str_size;
  char buf[32];

#ifdef __MINGW32__
  TIME_ZONE_INFORMATION TimeZoneInfo;
  GetTimeZoneInformation(&TimeZoneInfo);
  int hourOffset = TimeZoneInfo.Bias / 60;
  int minOffset = TimeZoneInfo.Bias % 60;
  bool posOffset = false;
  if (hourOffset < 0) {
    hourOffset *= -1;
    posOffset = true;
  }
  if (minOffset < 0) {
    minOffset *= -1;
  }

  char tz[5];
  sprintf(tz, "%s%02i:%02i", posOffset ? "+" : "-", hourOffset, minOffset);
  sprintf(buf, "%02i-%02i-%02iT%02i:%02i:%02i.%03lli%s", localt.tm_year + 1900, localt.tm_mon, localt.tm_mday,
          localt.tm_hour, localt.tm_min, localt.tm_sec, _last_ms.count() % 1000,
          tz);
  str_size = strlen(buf);
#else
  str_size = std::strftime(buf, sizeof(buf), "%FT%T.000%z0", &localt);
#endif

  string_ str(buf, str_size);
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
