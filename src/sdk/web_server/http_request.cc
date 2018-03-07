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
      _req(std::move(req)) {}

void HttpRequest::send_bad_response(http::status status,
                                    std::string const &error) {
  create_response<HttpStringResponse>();
  get_response<HttpStringResponse>()->init_response();
  get_response<HttpStringResponse>()->prep_response()->result(status);
  get_response<HttpStringResponse>()->prep_response()->keep_alive(false);
  get_response<HttpStringResponse>()->prep_response()->set(http::field::server,
                                                           "dglux_server");
  get_response<HttpStringResponse>()->prep_response()->set(
      http::field::content_type, "text/plain");
  get_response<HttpStringResponse>()->prep_response()->body() = error;
  get_response<HttpStringResponse>()->prep_response()->prepare_payload();
  get_response<HttpStringResponse>()->prep_serializer();
  get_response<HttpStringResponse>()->writer(std::move(_socket));
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
  create_response<HttpFileResponse>();
  get_response<HttpFileResponse>()->init_response();
  get_response<HttpFileResponse>()->prep_response()->result(http::status::ok);
  get_response<HttpFileResponse>()->prep_response()->keep_alive(false);
  get_response<HttpFileResponse>()->prep_response()->set(http::field::server,
                                                         "dglux_server");
  get_response<HttpFileResponse>()->prep_response()->set(
      http::field::content_type, mime_type(full_path));
  get_response<HttpFileResponse>()->prep_response()->body() = std::move(file);
  get_response<HttpFileResponse>()->prep_response()->prepare_payload();
  get_response<HttpFileResponse>()->prep_serializer();
  get_response<HttpFileResponse>()->writer(std::move(_socket));
}

void HttpRequest::redirect_handler(const string_ &location,
                                   const string_ &message) {
  create_response<HttpStringResponse>();
  get_response<HttpStringResponse>()->init_response();
  get_response<HttpStringResponse>()->prep_response()->result(
      http::status::found);
  get_response<HttpStringResponse>()->prep_response()->keep_alive(false);
  get_response<HttpStringResponse>()->prep_response()->set(
      http::field::location, location);
  get_response<HttpStringResponse>()->prep_response()->set(http::field::server,
                                                           "dglux_server");
  get_response<HttpStringResponse>()->prep_response()->set(
      http::field::content_type, "text/plain");
  get_response<HttpStringResponse>()->prep_response()->body() = message;
  get_response<HttpStringResponse>()->prep_response()->prepare_payload();
  get_response<HttpStringResponse>()->prep_serializer();
  get_response<HttpStringResponse>()->writer(std::move(_socket));
}

void HttpRequest::not_found_handler(const string_ &error) {
  send_bad_response(http::status::not_found, error);
}

void HttpRequest::file_server_handler(const string_ &_target) {
  // check session DGSESSION if exists and not timed out
  if (!is_authenticated())
    return redirect_handler("/login", "Login required to access this path");
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
  // TODO if (kvMap.at("b64") == "yes") use base64_decode for password
  if (!is_authenticated(kvMap.at("username"),
                        base64_decode(kvMap.at("password"))))
    return not_found_handler("Login did not succeed");
  // check session
  // create new session DGSESSION
  // if remember me create DGUSER DGTOKEN
  // response set cookie
  create_response<HttpStringResponse>();
  get_response<HttpStringResponse>()->init_response();
  get_response<HttpStringResponse>()->prep_response()->result(
      http::status::found);
  get_response<HttpStringResponse>()->prep_response()->keep_alive(false);
  get_response<HttpStringResponse>()->prep_response()->set(
      http::field::location, _doc_root);
  get_response<HttpStringResponse>()->prep_response()->set(http::field::server,
                                                           "dglux_server");
  get_response<HttpStringResponse>()->prep_response()->set(
      http::field::content_type, "text/plain");
  get_response<HttpStringResponse>()->prep_response()->set(
      http::field::cookie,
      _web_server.session_manager()->TOKEN_COOKIE + "=" + token);
  get_response<HttpStringResponse>()->prep_response()->body() =
      "Login succeeded";
  get_response<HttpStringResponse>()->prep_response()->prepare_payload();
  get_response<HttpStringResponse>()->prep_serializer();
  get_response<HttpStringResponse>()->writer(std::move(_socket));
}

bool HttpRequest::is_authenticated(const string_ &username,
                                   const string_ &password) {
  auto cvMap = parse(_req.at(http::field::cookie).to_string(), ";");
  auto token_cookie = cvMap.find(_web_server.session_manager()->TOKEN_COOKIE);
  bool authenticated = false;
  if (token_cookie == cvMap.end()) {
    _web_server.login_manager()->check_login(
        username, password, [&](const ClientInfo client, bool error) {
          // token = client.permission_str;
          authenticated = client.default_queue_size;  // TODO check
        });
  }
  return authenticated;
}

bool HttpRequest::is_authenticated() {
  auto cvMap = parse(_req.at(http::field::cookie).to_string(), ";");
  auto token_cookie = cvMap.find(_web_server.session_manager()->TOKEN_COOKIE);
  bool authenticated = false;
  if (token_cookie == cvMap.end())
    authenticated = _web_server.session_manager()->check_session(
        cvMap.at(_web_server.session_manager()->TOKEN_COOKIE));
  return authenticated;
}

bool HttpRequest::is_session_active() {
  auto cvMap = parse(_req.at(http::field::cookie).to_string(), ";");
  auto session_cookie =
      cvMap.find(_web_server.session_manager()->SESSION_COOKIE);
  bool active = false;
  if (session_cookie != cvMap.end()) active = true;
  //        _web_server.session_manager()->check_session(cvMap.at("DGSESSION"));

  return active;
}

void HttpRequest::create_session() {
  string_ session_cookie = generate_random_string(50);
  // if 'remember_me' feature is activated for the user in the users.json file
  // generate token_cookie = generate_random_string(80);
  // and user_cookie = "username will be taken from users.json file like
  // 'dgSuper'";
  if (!_web_server.session_manager()->check_session(session_cookie))
    _web_server.session_manager()->add_session(session_cookie);
  else
    create_session();
}
}