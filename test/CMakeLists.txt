if (UNIX)
	# Download and unpack googletest-download at configure time
	configure_file(CMakeLists.txt.in lib/googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/lib/googletest-download )
	if(result)
		message(FATAL_ERROR "CMake step for googletest failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test/lib/googletest-download )
	if(result)
		message(FATAL_ERROR "Build step for googletest failed: ${result}")
	endif()

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest-download directly to our build. This defines
	# the gtest and gtest_main targets.
	add_subdirectory(${CMAKE_BINARY_DIR}/test/lib/googletest-src
			${CMAKE_BINARY_DIR}/test/lib/googletest-build)

	# The gtest/gtest_main targets carry header search path
	# dependencies automatically when using CMake 2.8.11 or
	# later. Otherwise we have to add them here ourselves.
	#if (CMAKE_VERSION VERSION_LESS 2.8.11)
	include_directories("${gtest_SOURCE_DIR}/include")
	#endif()
endif (UNIX)

if (WIN32)
	find_package(GTest REQUIRED)
	include_directories(${GTEST_INCLUDE_DIRS})
	set(TEST_LIBS ${GTEST_LIBRARIES})
endif (WIN32)

find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time)
include_directories(${Boost_INCLUDE_DIRS})
set(TEST_LIBS ${TEST_LIBS} ${Boost_LIBRARIES})

include_directories("../src/sdk")
include_directories("../include")

#
add_subdirectory("./sdk")

# TESTS
set(TEST_LIBS ${TEST_LIBS} gtest_main dsa_sdk)
set(TARGET_NAME "dsa-test") 

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} ${TEST_LIBS})

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})