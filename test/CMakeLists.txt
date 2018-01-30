
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
set(TEST_LIBS dsa_sdk ${GTEST_BOTH_LIBRARIES} ${TEST_LIBS})

set(TARGET_NAME "dsa-test")

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} ${TEST_LIBS})

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})


# broker tests
set(BROKER_TEST_LIBS ${TEST_LIBS} dsa_broker)
include(./broker/CMakeListsInclude.txt)

# dslink tests
set(DSLINK_TEST_LIBS ${TEST_LIBS} dsa_broker)
include(./sdk/dslink/CMakeListsInclude.txt)

# broker-dslink tests
set(BROKER_DSLINK_TEST_LIBS ${TEST_LIBS} dsa_broker)
include(./broker_dslink/CMakeListsInclude.txt)
