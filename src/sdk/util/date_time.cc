#include "dsa_common.h"

#include "date_time.h"

#include <chrono>
#include <iomanip>

#ifdef __MINGW32__
#include <timezoneapi.h>
#include <cstring>
#endif

#if _MSC_VER
#define localtime_r(a, b) localtime_s(b, a)
#endif

namespace dsa {

static const size_t TS_SIZE = strlen("0000-00-00T00:00:00.000+00:00");

#ifndef __MINGW32__  // if not mingw

thread_local std::chrono::milliseconds _last_ms{0};
thread_local string_ _last_ts = "0000-00-00T00:00:00.000+00:00";

static void update_ts(std::chrono::system_clock::time_point now) {
  std::time_t t = std::chrono::system_clock::to_time_t(now);

  tm localt;
  localtime_r(&t, &localt);
  size_t str_size;
  char buf[32];

  str_size = std::strftime(buf, sizeof(buf), "%FT%T.000%z0", &localt);

  // string_ str(buf, str_size);

  // set milli second
  auto ms = _last_ms.count() % 1000;
  buf[20] = static_cast<char>((ms / 100) + '0');
  buf[21] = static_cast<char>(((ms % 100) / 10) + '0');
  buf[22] = static_cast<char>((ms % 10) + '0');

  // fix format of timezone;
  buf[28] = buf[27];
  buf[27] = buf[26];
  buf[26] = ':';

  std::copy(buf, buf + TS_SIZE, _last_ts.begin());
}

#else  // strftime doesn't work in mingw

static std::chrono::milliseconds _last_ms{0};
static string_ _last_ts = "0000-00-00T00:00:00.000+00:00";

// this is not 100% thread safe, might generate wrong timestamp in certain case
// but won't crash the application
static void update_ts(std::chrono::system_clock::time_point now) {
  std::time_t t = std::chrono::system_clock::to_time_t(now);

  tm localt;
  localtime_r(&t, &localt);
  size_t str_size;
  char buf[32];

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
  sprintf(buf, "%02i-%02i-%02iT%02i:%02i:%02i.%03lli%s", localt.tm_year + 1900,
          (localt.tm_mon + 1), localt.tm_mday, localt.tm_hour, localt.tm_min,
          localt.tm_sec,
          std::chrono::duration_cast<std::chrono::milliseconds>(
              now.time_since_epoch())
                  .count() %
              1000,
          tz);

  std::copy(buf, buf + TS_SIZE, _last_ts.begin());
}

#endif

const string_& DateTime::get_ts() {
  // only generate the string when time changed
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch());
  if (ms > _last_ms) {
    _last_ms = ms;
    update_ts(now);
  }
  return _last_ts;
}

int64_t DateTime::ms_since_epoch() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             now.time_since_epoch())
      .count();
}
}
