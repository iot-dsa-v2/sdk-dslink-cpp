#include "dsa_common.h"

#include "strand_timer.h"

namespace dsa {

StrandTimer::StrandTimer(LinkStrandRef&& strand, int32_t interval_ms,
                         LinkStrand::TimerCallback&& callback)
    : _strand(std::move(strand)),
      interval_ms(interval_ms),
      callback(callback),
      _timer(new boost::asio::deadline_timer(_strand->get_io_context())) {
  if (interval_ms > 0) {
    schedule(get_ref());
  }
}

void StrandTimer::destroy_impl() {
  if (callback != nullptr) {
    callback(true);
    callback = nullptr;
  }
  interval_ms = 0;
}

void StrandTimer::schedule(ref_<StrandTimer>&& rthis) {
  _timer->expires_from_now(boost::posix_time::milliseconds(interval_ms));
  _timer->async_wait([ this, keepref = get_ref(), rthis = std::move(rthis) ](
      const boost::system::error_code& error) mutable {
    _strand->dispatch([
      this, rthis = std::move(rthis),
      canceled = (error == boost::asio::error::operation_aborted)
    ]() mutable {
      if (callback != nullptr) {
        bool repeat = callback(canceled);
        if (repeat && interval_ms > 0) {
          schedule(std::move(rthis));
        }
      }
    });
  });
}
}
