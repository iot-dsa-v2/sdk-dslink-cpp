include_directories(${PROJECT_INCLUDE_DIR}/dsa/network)

find_package(Threads)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
  set(BOOST_INCLUDEDIR "/usr/local/Cellar/boost/1.64.0_1/include")
  set(BOOST_LIBRARYDIR "/usr/local/Cellar/boost/1.64.0_1/lib")
endif()
find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time)
include_directories(${Boost_INCLUDE_DIRS})

add_library(dsa_network OBJECT
  connection.cc 
  session.cc 
  tcp_connection.cc 
  tcp_server.cc)
  
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES} dsa_network)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_network> PARENT SCOPE)