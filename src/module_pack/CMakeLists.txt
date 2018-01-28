cmake_minimum_required(VERSION 3.7)

project(DSA_MODULE_PACK)

include_directories(${Boost_INCLUDE_DIRS})

set(MODULE_PACK_LIBS ${MODULE_PACK_LIBS} ${Boost_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})
set(MODULE_PACK_LIBS ${MODULE_PACK_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../sdk")

#
if (UNIX)
    set(MODULE_PACK_LIBS dsa_sdk ${MODULE_PACK_LIBS} dl)
else ()
    set(MODULE_PACK_LIBS dsa_sdk ${MODULE_PACK_LIBS})
endif ()


set(SOURCES logger/file_logger.cc)

add_library(dsa_module_file_logger SHARED ${SOURCES})
target_link_libraries(dsa_module_file_logger ${MODULE_PACK_LIBS})

