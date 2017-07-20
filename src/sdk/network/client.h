#ifndef DSA_SDK_NETWORK_CLIENT_H_
#define DSA_SDK_NETWORK_CLIENT_H_

#include "connection.h"

namespace dsa {

using Client = Connection;
typedef std::shared_ptr<Client> ClientPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CLIENT_H_
