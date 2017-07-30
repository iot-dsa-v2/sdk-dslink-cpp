include_directories(${LIB_RESPONDER})

list(APPEND ResponderSourceFiles
#        responder.cc
        node_model_manager.cc
        node_model.cc
        node_state_manager.cc
        node_state.cc
        outgoing_message_stream.cc)

foreach(file ${ResponderSourceFiles})
    list(APPEND ProjectSourceFiles ${LIB_RESPONDER}/${file})
endforeach()
