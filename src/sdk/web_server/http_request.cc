#include "http_request.h"
#include <regex>
#include "crypto/misc.h"
#include "http_response.h"
#include "login_manager.h"
#include "module/default/simple_login_manager.h"
#include "util/string.h"

namespace {

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

std::string path_cat(boost::beast::string_view base,
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

std::map<std::string, std::string> parse(const std::string &query,
                                         const std::string separator) {
  std::map<std::string, std::string> data;
  std::regex pattern("([\\w+%]+)=([^" + separator + "]*)");
  auto words_begin = std::sregex_iterator(query.begin(), query.end(), pattern);
  auto words_end = std::sregex_iterator();

  for (std::sregex_iterator i = words_begin; i != words_end; i++) {
    std::string key = (*i)[1].str();
    std::string value = (*i)[2].str();
    data[key] = value;
  }

  return data;
}
}

namespace dsa {

HttpRequest::HttpRequest(
    WebServer &web_server, boost::asio::ip::tcp::socket socket,
    http::request<request_body_t, http::basic_fields<alloc_t>> req)
    : _web_server(web_server),
      _socket(std::move(socket)),
      _req(std::move(req)),
      _resp(std::make_shared<HttpResponse>()) {}

shared_ptr_<HttpResponse> HttpRequest::get_response() { return _resp; }

void HttpRequest::send_bad_response(http::status status,
                                    std::string const &error) {
  shared_ptr_<HttpResponse> response = get_response();
  response->prepare_string_response();
  response->get_string_response()->result(status);
  response->get_string_response()->keep_alive(false);
  response->get_string_response()->set(http::field::server, "dglux_server");
  response->get_string_response()->set(http::field::content_type, "text/plain");
  response->get_string_response()->body() = error;
  response->get_string_response()->prepare_payload();
  response->prepare_string_serializer();
  response->string_writer(std::move(_socket));
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

  shared_ptr_<HttpResponse> response = get_response();
  response->prepare_file_response();
  response->get_file_response()->result(http::status::ok);
  response->get_file_response()->keep_alive(false);
  response->get_file_response()->set(http::field::server, "dglux_server");
  response->get_file_response()->set(http::field::content_type,
                                     mime_type(full_path));
  response->get_file_response()->body() = std::move(file);
  response->get_file_response()->prepare_payload();
  response->prepare_file_serializer();
  response->file_writer(std::move(_socket));
}

void HttpRequest::redirect_handler(const string_ &location,
                                   const string_ &message) {
  shared_ptr_<HttpResponse> response = get_response();
  response->prepare_string_response();
  response->get_string_response()->result(http::status::found);
  response->get_string_response()->keep_alive(false);
  response->get_string_response()->set(http::field::location, location);
  response->get_string_response()->set(http::field::server, "dglux_server");
  response->get_string_response()->set(http::field::content_type, "text/plain");
  response->get_string_response()->body() = message;
  response->get_string_response()->prepare_payload();
  response->prepare_string_serializer();
  response->string_writer(std::move(_socket));
}

void HttpRequest::not_found_handler(const string_ &error) {
  send_bad_response(http::status::not_found, error);
}

void HttpRequest::file_server_handler(const string_ &_target) {
  // check session DGSESSION if exists and not timed out
  // else redirect to login page
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
  auto kvMap = parse(boost::beast::buffers_to_string(_req.body().data()), "&");
  string_ token;
  // if (kvMap.at("b64") == "yes") use base64_decode for password
  if (!is_authenticated(kvMap.at("username"),
                        base64_decode(kvMap.at("password"))))
    return not_found_handler("Login did not succeed");
  // check session
  // create new session DGSESSION
  // if remember me create DGUSER DGTOKEN
  // response set cookie
  shared_ptr_<HttpResponse> response = get_response();
  response->prepare_string_response();
  response->get_string_response()->result(http::status::found);
  response->get_string_response()->keep_alive(false);
  response->get_string_response()->set(http::field::location, _doc_root);
  response->get_string_response()->set(http::field::server, "dglux_server");
  response->get_string_response()->set(http::field::content_type, "text/plain");
  response->get_string_response()->body() = "Login succeeded";
  response->get_string_response()->prepare_payload();
  response->set_cookie("DGTOKEN=" + token);
  response->prepare_string_serializer();
  response->string_writer(std::move(_socket));
}

bool HttpRequest::is_authenticated(const string_ &username,
                                   const string_ &password) {
  auto cvMap = parse(_req.at(http::field::cookie).to_string(), ";");
  auto dgtoken = cvMap.find("DGTOKEN");
  bool authenticated = false;
  if (dgtoken == cvMap.end())
  {
    _web_server.login_manager()->check_login(
        username, password,
        [&](const ClientInfo client, bool error) {
          //token = client.permission_str;
          authenticated = client.default_queue_size;  // TODO check
        });
  }
  return authenticated;
}

bool HttpRequest::is_session_active(const string_ &session_id) {
  auto cvMap = parse(_req.at(http::field::cookie).to_string(), ";");
  auto dgsession = cvMap.find("DGSESSION");
  bool active = false;
  if (dgsession != cvMap.end())
    active = true;
//        _web_server.session_manager()->check_session(cvMap.at("DGSESSION"));

  return active;
}
void HttpRequest::create_session() {
  string_ token = generate_random_string(50);
//  _web_server.session_manager()->add_session(token);
}

}