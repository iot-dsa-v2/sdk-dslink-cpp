find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time program_options)
include_directories(${Boost_INCLUDE_DIRS})
set(BROKER_LIBS ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(BROKER_LIBS ${BROKER_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../sdk")
include_directories("../../include")

#
if (UNIX)
    set(BROKER_LIBS dsa_sdk ${BROKER_LIBS} dl)
else ()
    set(BROKER_LIBS dsa_sdk ${BROKER_LIBS})
endif ()

