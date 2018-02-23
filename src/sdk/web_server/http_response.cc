#include "http_response.h"

namespace dsa {

HttpResponse::HttpResponse() = default;

void HttpResponse::prepare_file_response() {
  _file_resp.emplace(std::piecewise_construct, std::make_tuple(),
                     std::make_tuple(_alloc));
}
void HttpResponse::prepare_file_serializer() {
  _file_serializer.emplace(*_file_resp);
}
void HttpResponse::prepare_string_response() {
  _str_resp.emplace(std::piecewise_construct, std::make_tuple(),
                    std::make_tuple(_alloc));
}
void HttpResponse::prepare_string_serializer() {
  _str_serializer.emplace(*_str_resp);
}

void HttpResponse::file_writer(tcp::socket&& _socket) {
  http::async_write(_socket, *_file_serializer,
                    [this, &_socket](boost::beast::error_code ec, std::size_t) {
                      _socket.shutdown(tcp::socket::shutdown_send, ec);
                      _file_serializer.reset();
                      _file_resp.reset();
                    });
}
void HttpResponse::string_writer(tcp::socket&& _socket) {
  http::async_write(_socket, *_str_serializer,
                    [this, &_socket](boost::beast::error_code ec, std::size_t) {
                      _socket.shutdown(tcp::socket::shutdown_send, ec);
                      _str_serializer.reset();
                      _str_resp.reset();
                    });
}

}