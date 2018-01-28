
include_directories(${Boost_INCLUDE_DIRS})
set(TEST_LIBS ${TEST_LIBS} ${Boost_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})
set(TEST_LIBS ${TEST_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../src/sdk")
include_directories("../src/broker")
include_directories("../include")

#
add_subdirectory("./sdk")

# TESTS
set(TEST_LIBS gtest_main dsa_sdk ${TEST_LIBS})


set(TARGET_NAME "dsa-test")

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} ${TEST_LIBS})

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})



add_test(NAME "test-broker-dslink" COMMAND "test-broker-dslink")

add_executable("dslink-test" sdk/dslink/dslink-config-test.cc
                             sdk/dslink/dslink-subscribe-test.cc
                             sdk/dslink/dslink-list-test.cc
                             sdk/dslink/dslink-server-test.cc
                             sdk/dslink/dslink-invoke-test.cc
                             sdk/dslink/dslink-connect-test.cc
                             sdk/test_config.cc sdk/async_test.cc
                             broker/util/broker_runner.cc)

# broker tests
set(BROKER_TEST_LIBS ${TEST_LIBS} dsa_broker)
include(./broker/CMakeListsInclude.txt)

target_link_libraries("dslink-test" ${TEST_LIBS} dsa_broker)
add_test(NAME "dslink-test" COMMAND "dslink-test")

add_executable("broker-dslink-test"
        broker_dslink/broker_dslink_test.cc
        broker_dslink/broker_dslink_list.cc
        broker_dslink/broker_dslink_util.cc
        sdk/async_test.cc
        sdk/test_config.cc)
target_link_libraries("broker-dslink-test" ${TEST_LIBS} dsa_broker)
