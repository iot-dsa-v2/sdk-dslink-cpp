#include "dsa_common.h"

#include "sys_root.h"

#include "../../broker.h"
#include "module/logger.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"

namespace dsa {

BrokerSysRoot::BrokerSysRoot(LinkStrandRef &&strand, ref_<DsBroker> &&broker)
    : NodeModel(std::move(strand)) {
  add_list_child(
      "stop",
      make_ref_<SimpleInvokeNode>(
          _strand->get_ref(),
          [broker = std::move(broker)](Var && v, SimpleInvokeNode & node,
                                       OutgoingInvokeStream & stream) {
            // Checking Token
            // if (v.get_type() == Var::STRING &&
            //   broker->get_close_token() == v.get_string()) {
            auto timer = make_unique_<boost::asio::deadline_timer>(
                broker->strand->get_io_context(),
                boost::posix_time::seconds(1));
            // keep raw pointer because of std::move
            auto temp_p_timer = timer.get();
            temp_p_timer->async_wait(
                [broker](const boost::system::error_code &error) mutable {
                  if (error != boost::asio::error::operation_aborted) {
                    broker->strand->post([broker]() {
                      LOG_SYSTEM(broker.get()->strand.get()->logger(),
                                 LOG << "DsBroker stopped");
                      broker->destroy();
                    });
                  }
                });

            stream.close();
            // make sure the close message is sent asap
            stream.make_critical();
            //} else {
            //  stream.close(MessageStatus::INVALID_PARAMETER);
            //}
          },
          PermissionLevel::CONFIG));
}
}
