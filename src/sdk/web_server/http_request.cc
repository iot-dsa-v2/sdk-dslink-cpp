#include "http_request.h"
#include <regex>
#include "crypto/misc.h"

namespace dsa {

boost::beast::string_view mime_type(boost::beast::string_view path) {
  using boost::beast::iequals;
  auto const ext = [&path] {
    auto const pos = path.rfind(".");
    if (pos == boost::beast::string_view::npos)
      return boost::beast::string_view{};
    return path.substr(pos);
  }();
  if (iequals(ext, ".htm")) return "text/html";
  if (iequals(ext, ".html")) return "text/html";
  if (iequals(ext, ".php")) return "text/html";
  if (iequals(ext, ".css")) return "text/css";
  if (iequals(ext, ".txt")) return "text/plain";
  if (iequals(ext, ".js")) return "application/javascript";
  if (iequals(ext, ".json")) return "application/json";
  if (iequals(ext, ".xml")) return "application/xml";
  if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
  if (iequals(ext, ".flv")) return "video/x-flv";
  if (iequals(ext, ".png")) return "image/png";
  if (iequals(ext, ".jpe")) return "image/jpeg";
  if (iequals(ext, ".jpeg")) return "image/jpeg";
  if (iequals(ext, ".jpg")) return "image/jpeg";
  if (iequals(ext, ".gif")) return "image/gif";
  if (iequals(ext, ".bmp")) return "image/bmp";
  if (iequals(ext, ".ico")) return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff")) return "image/tiff";
  if (iequals(ext, ".tif")) return "image/tiff";
  if (iequals(ext, ".svg")) return "image/svg+xml";
  if (iequals(ext, ".svgz")) return "image/svg+xml";
  return "application/text";
}

string_ path_cat(boost::beast::string_view base,
                 boost::beast::string_view path) {
  if (base.empty()) return path.to_string();
  std::string result = base.to_string();
#if BOOST_MSVC
  char constexpr path_separator = '\\';
  if (result.back() == path_separator) result.resize(result.size() - 1);
  result.append(path.data(), path.size());
  for (auto &c : result)
    if (c == '/') c = path_separator;
#else
  char constexpr path_separator = '/';
  if (result.back() == path_separator) result.resize(result.size() - 1);
  result.append(path.data(), path.size());
#endif
  return result;
}

std::map<std::string, std::string> parse(const std::string &query) {
  std::map<std::string, std::string> data;
  std::regex pattern("([\\w+%]+)=([^&]*)");
  auto words_begin = std::sregex_iterator(query.begin(), query.end(), pattern);
  auto words_end = std::sregex_iterator();

  for (std::sregex_iterator i = words_begin; i != words_end; i++) {
    std::string key = (*i)[1].str();
    std::string value = (*i)[2].str();
    data[key] = value;
  }

  return data;
}

HttpRequest::HttpRequest(
    WebServer &web_server, boost::asio::ip::tcp::socket socket,
    http::request<request_body_t, http::basic_fields<alloc_t>> req)
    : _web_server(web_server),
      _socket(std::move(socket)),
      _req(std::move(req)) {}

void HttpRequest::send_bad_response(http::status status,
                                    std::string const &error) {
  _str_resp.emplace(std::piecewise_construct, std::make_tuple(),
                    std::make_tuple(_alloc));

  _str_resp->result(status);
  _str_resp->keep_alive(false);
  _str_resp->set(http::field::server, "dglux_server");
  _str_resp->set(http::field::content_type, "text/plain");
  _str_resp->body() = error;
  _str_resp->prepare_payload();

  _str_serializer.emplace(*_str_resp);

  string_writer();
}

void HttpRequest::send_file(boost::beast::string_view target) {
  if (target.empty() || target[0] != '/' ||
      target.find("..") != std::string::npos) {
    send_bad_response(http::status::not_found, "File not found\r\n");
    return;
  }

  std::string full_path = _doc_root;
  full_path.append(target.data(), target.size());

  http::file_body::value_type file;
  boost::beast::error_code ec;
  file.open(full_path.c_str(), boost::beast::file_mode::read, ec);
  if (ec) {
    send_bad_response(http::status::not_found, "File not found\r\n");
    return;
  }

  _file_resp.emplace(std::piecewise_construct, std::make_tuple(),
                     std::make_tuple(_alloc));

  _file_resp->result(http::status::ok);
  _file_resp->keep_alive(false);
  _file_resp->set(http::field::server, "dglux_server");
  _file_resp->set(http::field::content_type, mime_type(target.to_string()));
  _file_resp->body() = std::move(file);
  _file_resp->prepare_payload();

  _file_serializer.emplace(*_file_resp);

  file_writer();
}

void HttpRequest::string_writer() {
  http::async_write(_socket, *_str_serializer,
                    [this](boost::beast::error_code ec, std::size_t) {
                      _socket.shutdown(tcp::socket::shutdown_send, ec);
                      _str_serializer.reset();
                      _str_resp.reset();
                    });
}

void HttpRequest::file_writer() {
  http::async_write(_socket, *_file_serializer,
                    [this](boost::beast::error_code ec, std::size_t) {
                      _socket.shutdown(tcp::socket::shutdown_send, ec);
                      _file_serializer.reset();
                      _file_resp.reset();
                    });
}

void HttpRequest::redirect_handler(const string_ &location,
                                   const string_ &message) {
  _str_resp.emplace(std::piecewise_construct, std::make_tuple(),
                    std::make_tuple(_alloc));

  _str_resp->result(http::status::found);
  _str_resp->keep_alive(false);
  _str_resp->set(http::field::location, location);
  _str_resp->set(http::field::server, "dglux_server");
  _str_resp->set(http::field::content_type, "text/plain");
  _str_resp->body() = message;
  _str_resp->prepare_payload();

  _str_serializer.emplace(*_str_resp);

  string_writer();
}

void HttpRequest::not_found_handler(const string_ &error) {
  send_bad_response(http::status::not_found, error);
}

void HttpRequest::file_server_handler(const string_ &_target) {
  switch (_req.method()) {
    case http::verb::get:
      send_file(_req.target());
      break;

    default:
      send_bad_response(http::status::bad_request,
                        "Invalid request-method '" +
                            _req.method_string().to_string() + "'\r\n");
      break;
  }
}
void HttpRequest::authentication_handler() {
  auto kvMap = parse(boost::beast::buffers_to_string(_req.body().data()));
  if (kvMap.at("username") == "dgSuper" &&
      base64_decode(kvMap.at("password")) == "dglux1234") {
    std::cout << "User logged in successfully" << std::endl;
    redirect_handler("/", "Login succeeded");
  }
}
}