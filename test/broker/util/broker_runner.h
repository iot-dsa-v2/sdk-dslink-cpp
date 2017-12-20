#ifndef BROKER_TEST_BROKER_RUNNER_H
#define BROKER_TEST_BROKER_RUNNER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "broker.h"

namespace dsa {
ref_<DsBroker> create_broker(std::shared_ptr<App> app = nullptr);
WrapperStrand get_client_wrapper_strand(const ref_<DsBroker> &broker,
                                        const string_ &dsid_prefix = "test");
}

#endif  // BROKER_TEST_BROKER_RUNNER_H
