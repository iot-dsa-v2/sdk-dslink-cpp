find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time)
include_directories(${Boost_INCLUDE_DIRS})
set(BENCHMARK_LIBS ${BENCHMARK_LIBS} ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(BENCHMARK_LIBS ${BENCHMARK_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../src/sdk")
include_directories("../include")

#
if (UNIX)
    set(BENCHMARK_LIBS dsa_sdk ${BENCHMARK_LIBS} dl)
else ()
    set(BENCHMARK_LIBS dsa_sdk ${BENCHMARK_LIBS})
endif ()

set(TARGET_NAME "dsa-benchmark")
set(SOURCES "benchmark.cc" $<TARGET_OBJECTS:dsa_test_util>)

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} ${BENCHMARK_LIBS})