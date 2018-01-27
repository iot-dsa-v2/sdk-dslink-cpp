include_directories(${Boost_INCLUDE_DIRS})
set(BENCHMARK_LIBS ${BENCHMARK_LIBS} ${Boost_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})
set(BENCHMARK_LIBS ${BENCHMARK_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../src/sdk")
include_directories("../include")

#
set(BENCHMARK_LIBS dsa_sdk ${BENCHMARK_LIBS})


#
foreach (name "throughput"
#              "throughput_server"
#              "throughput_client"
#              "message_queue"
              "broker_throughput"
)
    set(SOURCES "${name}.cc" $<TARGET_OBJECTS:dsa_test_util>)
    add_executable(${name} ${SOURCES})
    target_link_libraries(${name} ${BENCHMARK_LIBS})
endforeach()
