include_directories(${Boost_INCLUDE_DIRS})
set(BROKER_LIBS ${BROKER_LIBS} ${Boost_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})
set(BROKER_LIBS ${BROKER_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../sdk")

#
if (UNIX)
    set(BROKER_LIBS dsa_sdk ${BROKER_LIBS} dl)
else ()
    set(BROKER_LIBS dsa_sdk ${BROKER_LIBS})
endif ()


set(SOURCES broker_main.cc
        broker.cc
        config/broker_config.cc
        config/broker_config_item.cc
        config/module_loader.cc
        module/broker_security_manager.cc
        remote_node/broker_session_manager.cc
        remote_node/broker_client.cc
        remote_node/remote_node.cc
        remote_node/remote_root_node.cc
        remote_node/invoke_proxy.cc
        remote_node/set_proxy.cc
        node/broker_root.cc
        node/downstream/downstream_root.cc
        node/upstream/upstream_root.cc
        node/home/home_root.cc
        node/sys/sys_root.cc
        node/pub/pub_root.cc)

add_library(dsa_broker ${SOURCES})
target_link_libraries(dsa_broker ${BROKER_LIBS})

set(BROKER_LIBS ${BROKER_LIBS} dsa_broker)

add_executable(broker broker_main.cc)
target_link_libraries(broker ${BROKER_LIBS})
