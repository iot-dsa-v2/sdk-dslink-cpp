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
void HttpResponse::file_writer(Websocket&& websocket) {
  websocket.async_write(*_file_serializer,
                    [this, &websocket](boost::beast::error_code ec, std::size_t) {
                      websocket.shutdown(tcp::socket::shutdown_send, ec);
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
void HttpResponse::string_writer(Websocket&& websocket) {
  websocket.async_write(*_str_serializer,
                    [this, &websocket](boost::beast::error_code ec, std::size_t) {
                      websocket.shutdown(tcp::socket::shutdown_send, ec);
                      _str_serializer.reset();
                      _str_resp.reset();
                    });
}
boost::optional<
    http::response<boost::beast::http::basic_string_body<char>,
                   boost::beast::http::basic_fields<dsa::fields_alloc<char>>>>&
HttpResponse::get_string_response() {
  return _str_resp;
}
boost::optional<http::response<
    boost::beast::http::basic_file_body<boost::beast::file>,
    boost::beast::http::basic_fields<dsa::fields_alloc<char>>>>&
HttpResponse::get_file_response() {
  return _file_resp;
}
}
