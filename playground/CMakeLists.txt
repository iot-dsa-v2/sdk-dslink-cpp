
if (WIN32)
    find_package(GTest REQUIRED)
endif (WIN32)
include_directories(${GTEST_INCLUDE_DIRS})
set(TEST_LIBS ${GTEST_LIBRARIES})

include_directories(${Boost_INCLUDE_DIRS})
set(TEST_LIBS ${TEST_LIBS} ${Boost_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})
set(TEST_LIBS ${TEST_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../src/sdk")
include_directories("../src/broker")
include_directories("../include")


# TESTS
if (UNIX)
    set(TEST_LIBS gtest_main dsa_sdk ${TEST_LIBS} dl)
else ()
    set(TEST_LIBS gtest_main dsa_sdk ${TEST_LIBS})
endif ()

set(TARGET_NAME "playground")

add_executable(${TARGET_NAME} playground.cc ../test/sdk/async_test.cc ../test/sdk/test_config.cc)
target_link_libraries(${TARGET_NAME} ${TEST_LIBS} dsa_broker)

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
