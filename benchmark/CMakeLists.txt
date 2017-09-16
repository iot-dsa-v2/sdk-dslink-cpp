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


set(SOURCES_benchmark "benchmark.cc" $<TARGET_OBJECTS:dsa_test_util>)
add_executable("dsa-benchmark" ${SOURCES_benchmark})
target_link_libraries("dsa-benchmark" ${BENCHMARK_LIBS})

set(SOURCES_throughput "throughput.cc" $<TARGET_OBJECTS:dsa_test_util>)
add_executable("dsa_throughput" ${SOURCES_throughput})
target_link_libraries("dsa_throughput" ${BENCHMARK_LIBS})
