#include "dsa_common.h"

#include "node_model.h"

#include "node_state.h"

namespace dsa {

void NodeModel::update_value(Buffer::SharedBuffer buf) {
  if (_state != nullptr)
    _state->new_message(buf);
}

}  // namespace dsa