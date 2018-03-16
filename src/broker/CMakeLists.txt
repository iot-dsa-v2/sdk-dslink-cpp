project(DSA_BROKER)

include_directories(${Boost_INCLUDE_DIRS})
set(BROKER_LIBS ${BROKER_LIBS} ${Boost_LIBRARIES})

include_directories(${OPENSSL_INCLUDE_DIR})
set(BROKER_LIBS ${BROKER_LIBS} ${OPENSSL_LIBRARIES})

include_directories("../sdk")

#
set(BROKER_LIBS dsa_sdk ${BROKER_LIBS})



set(SOURCES
        broker.cc
        config/broker_config.cc
        config/broker_config_item.cc
        module/client/broker_client_manager.cc
        module/client/broker_client_manager_config.cc
        module/client/broker_client_nodes.cc
        module/authorizer/broker_authorizer.cc
        module/module_broker_default.cc
        remote_node/broker_session_manager.cc
        remote_node/broker_client.cc
        remote_node/remote_node.cc
        remote_node/remote_root_node.cc
        remote_node/invoke_proxy.cc
        remote_node/set_proxy.cc
        remote_node/dynamic_children_parent.cc
        node/broker_root.cc
        node/upstream/upstream_root.cc
        node/home/home_root.cc
        node/sys/sys_root.cc
        node/pub/pub_root.cc
        module/user/broker_login_manager.cc
        module/user/broker_login_manager.h
        module/client/quaratine_node.cpp
        module/client/quaratine_node.h)

add_library(dsa_broker ${SOURCES})
target_link_libraries(dsa_broker ${BROKER_LIBS})

set(BROKER_LIBS ${BROKER_LIBS} dsa_broker)

add_executable(broker broker_main.cc)
target_link_libraries(broker ${BROKER_LIBS})
