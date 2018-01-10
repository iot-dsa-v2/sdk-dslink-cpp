#ifndef DSA_SDK_STRAND_TIMER_H
#define DSA_SDK_STRAND_TIMER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "link_strand.h"

#include <boost/asio/deadline_timer.hpp>
#include <memory>

namespace dsa {
class StrandTimer final : public DestroyableRef<StrandTimer> {
  friend class LinkStrand;

 public:
  // change interval_ms won't affect the current waiting callback
  // it affects the next one when timer repeat
  int32_t repeat_interval_ms;
  // reschedule the timer, this won't affect the repeat interval
  void reschedule(int32_t interval_ms);
  void restart(int32_t interval_ms);
  bool is_running() {return _running;}


 protected:
  LinkStrandRef _strand;
  std::unique_ptr<boost::asio::deadline_timer> _timer;
  LinkStrand::TimerCallback _callback;
  bool _running = false;
  StrandTimer(LinkStrandRef&& strand, int32_t interval_ms,
              LinkStrand::TimerCallback&& callback);

  void schedule(ref_<StrandTimer>&& rthis, int32_t interval_ms);

  void destroy_impl() final;
};
}

#endif  // DSA_SDK_STRAND_TIMER_H
