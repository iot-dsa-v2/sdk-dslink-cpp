#ifndef BROKER_TEST_BROKER_RUNNER_H
#define BROKER_TEST_BROKER_RUNNER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "broker.h"

namespace dsa {
  shared_ptr_<DsBroker> create_broker();
}


#endif //BROKER_TEST_BROKER_RUNNER_H
