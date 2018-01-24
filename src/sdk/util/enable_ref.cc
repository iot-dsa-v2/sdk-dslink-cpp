#include "dsa_common.h"

#include "enable_ref.h"

thread_local int _dsa_ref_guard_count{0};
thread_local int _dsa_ref_guard_rand{0};
