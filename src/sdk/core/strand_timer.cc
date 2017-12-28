#include "dsa_common.h"

#include "strand_timer.h"

namespace dsa {

StrandTimer::StrandTimer(LinkStrandRef&& strand, int32_t interval,
                         LinkStrand::TimerCallback&& callback)
    : _strand(std::move(strand)),
      interval(interval),
      callback(callback),
      _timer(new boost::asio::deadline_timer(_strand->get_io_context())) {
  if (interval > 0) {
    _timer->expires_from_now(boost::posix_time::seconds(interval));
    _timer->async_wait(get_ref());
  }
}

void StrandTimer::destroy_impl() { callback = nullptr; }

void StrandTimer::operator()(const boost::system::error_code& error) {
  _strand->dispatch([
    this, keepref = get_ref(),
    canceled = error == boost::asio::error::operation_aborted
  ]() {
    if (callback != nullptr) {
      bool repeat = callback(canceled);
      if (repeat && interval > 0) {
        _timer->expires_from_now(boost::posix_time::seconds(interval));
        _timer->async_wait(get_ref());
      }
    }
  });
}
}
