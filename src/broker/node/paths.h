#ifndef DSA_SDK_PATHS_H
#define DSA_SDK_PATHS_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
const std::string DOWNSTREAM_NAME = "downstream";
const std::string DOWNSTREAM_PATH = DOWNSTREAM_NAME + "/";
const std::string PUB_PATH = "pub/";
const std::string QUARANTINE_PATH = "sys/quarantine/";
}  // namespace dsa

#endif  // DSA_SDK_PATHS_H
