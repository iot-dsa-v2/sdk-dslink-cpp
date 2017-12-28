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
  int32_t interval;
  LinkStrand::TimerCallback callback;
  StrandTimer(LinkStrandRef&& strand, int32_t interval,
              LinkStrand::TimerCallback&& callback);

  void operator()(const boost::system::error_code& error);

 protected:
  LinkStrandRef _strand;
  std::unique_ptr<boost::asio::deadline_timer> _timer;

  void destroy_impl() final;
};
}

#endif  // DSA_SDK_STRAND_TIMER_H
