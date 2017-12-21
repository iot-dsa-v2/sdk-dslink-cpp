find_package(Boost 1.66 REQUIRED COMPONENTS system thread date_time program_options filesystem)
include_directories(${Boost_INCLUDE_DIRS})
set(EXAMPLE_LIBS ${EXAMPLE_LIBS} ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(EXAMPLE_LIBS ${EXAMPLE_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../src/sdk")
include_directories("../include")

#
if (UNIX)
    set(EXAMPLE_LIBS dsa_sdk ${EXAMPLE_LIBS} dl)
else ()
    set(EXAMPLE_LIBS dsa_sdk ${EXAMPLE_LIBS})
endif ()


#
foreach (name "basic"
        )
    set(SOURCES "dslink/${name}.cc")
    add_executable(${name} ${SOURCES})
    target_link_libraries(${name} ${EXAMPLE_LIBS})
endforeach()
