#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#include <boost/asio.hpp>

#include "config.h"
#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class Connection;

class Client : public SharedClosable<Client> {
 protected:
  intrusive_ptr_<Session> _session;
  shared_ptr_<Connection> _connection;

  const Config _config;
  boost::asio::io_service::strand &_strand;

 public:
  Client(boost::asio::io_service::strand &strand, const Config &config);

  virtual void connect() = 0;

  void close() override;
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_
