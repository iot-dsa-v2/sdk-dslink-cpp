#include "dsa_common.h"

#include "strand_timer.h"

namespace dsa {

StrandTimer::StrandTimer(const LinkStrandRef &strand, int32_t interval_ms,
                         LinkStrand::TimerCallback&& callback)
    : _strand(strand),
      repeat_interval_ms(interval_ms),
      _callback(std::move(callback)),
      _timer(new boost::asio::deadline_timer(_strand->get_io_context())) {
  if (interval_ms > 0) {
    schedule(get_ref(), interval_ms);
  }
}

void StrandTimer::destroy_impl() {
  if (_callback != nullptr) {
    _callback(true);
    _callback = nullptr;
  }
  repeat_interval_ms = 0;
  _timer->cancel();
}

void StrandTimer::reschedule(int32_t interval_ms) {
  if (interval_ms > 0) {
    _timer->expires_from_now(boost::posix_time::milliseconds(interval_ms));
  }
}
void StrandTimer::restart(int32_t interval_ms) {
  if (interval_ms > 0 && _running == false) {
    schedule(get_ref(), interval_ms);
  }
}

void StrandTimer::schedule(ref_<StrandTimer>&& rthis, int32_t interval_ms) {
  _running = true;
  _timer->expires_from_now(boost::posix_time::milliseconds(interval_ms));
  _timer->async_wait([ this, rthis = std::move(rthis) ](
      const boost::system::error_code& error) mutable {
    DSA_REF_GUARD();
    _strand->dispatch([
      this, rthis = std::move(rthis),
      canceled = (error == boost::asio::error::operation_aborted)
    ]() mutable {
      if (_callback != nullptr) {
        _running = _callback(canceled);
        if (_running && repeat_interval_ms > 0) {
          schedule(std::move(rthis), repeat_interval_ms);
        }
      }
    });
  });
}
}
