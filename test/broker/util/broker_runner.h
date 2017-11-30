#ifndef BROKER_TEST_BROKER_RUNNER_H
#define BROKER_TEST_BROKER_RUNNER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../sdk/test_config.h"
#include "broker.h"

namespace dsa {
shared_ptr_<DsBroker> create_broker();
WrapperStrand get_client_wrapper_strand(const shared_ptr_<DsBroker> &broker,
                                        const string_ &dsid_prefix = "test");
}

#endif  // BROKER_TEST_BROKER_RUNNER_H
