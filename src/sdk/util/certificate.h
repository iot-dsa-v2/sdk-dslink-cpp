#ifndef DSA_CERTIFICATE_H
#define DSA_CERTIFICATE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ssl/stream.hpp>
namespace ssl = boost::asio::ssl;

namespace dsa {

void generate_certificate();
void load_root_certificate(ssl::context& context,
                           boost::system::error_code& error_code);
bool load_server_certificate(ssl::context& context,
                             boost::system::error_code& error_code);

}  // namespace dsa

#endif  // DSA_CERTIFICATE_H
