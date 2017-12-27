#include <iostream>
#include "dsa_common.h"

#include "sys_root.h"

#include "../link.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"
namespace dsa {

LinkSysRoot::LinkSysRoot(LinkStrandRef &&strand, ref_<DsLink> &&link)
    : NodeModel(std::move(strand)) {
  if (link->get_close_token() != "") {
    add_list_child(
        "stop", make_ref_<SimpleInvokeNode>(
                    _strand->get_ref(),
                    [link = std::move(link)](Var && v, SimpleInvokeNode & node,
                                             OutgoingInvokeStream & stream) {
                      // Checking Token
                      if (v.get_type() == Var::STRING &&
                          link->get_close_token() == v.get_string()) {
                        auto timer = make_unique_<boost::asio::deadline_timer>(
                            link->strand->get_io_context(),
                            boost::posix_time::seconds(1));
                        // keep raw pointer because of std::move
                        auto temp_p_timer = timer.get();
                        temp_p_timer->async_wait([link, timer = std::move(timer)](
                            const boost::system::error_code &error) mutable {
                          if (error != boost::asio::error::operation_aborted) {
                            link->strand->post([link]() {
                              link->destroy();
                            });
                          }
                        });
                        stream.close();
                        // make sure the close message is sent asap
                        stream.make_critical();
                      } else {
                        stream.close(MessageStatus::INVALID_PARAMETER);
                      }
                    },
                    PermissionLevel::CONFIG));
  }
}
LinkSysRoot::~LinkSysRoot() = default;
}
