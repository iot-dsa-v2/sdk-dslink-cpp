#ifndef DSA_SDK_PATHS_H
#define DSA_SDK_PATHS_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
const std::string DOWNSTREAM_NAME = "Downstream";
const std::string DOWNSTREAM_PATH = DOWNSTREAM_NAME + "/";
const std::string PUB_PATH = "Pub/";
const std::string QUARANTINE_PATH = "Sys/Quarantine/";
}  // namespace dsa

#endif  // DSA_SDK_PATHS_H
