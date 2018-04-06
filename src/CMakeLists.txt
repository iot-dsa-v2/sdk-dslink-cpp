project(sdk-dslink-cpp)

include_directories(${SDK_PROJECT_INCLUDE_DIR})

add_subdirectory("${PROJECT_SOURCE_DIR}/sdk")

if(DSA_INCLUDE_BROKER)
  add_subdirectory("${PROJECT_SOURCE_DIR}/broker")
endif()

if(DSA_INCLUDE_MODULE_PACK)
  add_subdirectory("${PROJECT_SOURCE_DIR}/module_pack")
endif()
