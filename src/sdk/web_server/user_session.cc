#include "dsa_common.h"

#include "crypto/misc.h"
#include "user_session.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <regex>
//#include "web_server.h"

namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

namespace dsa {

std::map<std::string, std::string> Parse(const std::string& query) {
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

UserSession::UserSession(
    WebServer& web_server, boost::asio::ip::tcp::socket socket,
    boost::beast::http::request<boost::beast::http::string_body> req)
    : _web_server(web_server),
      _socket(std::move(socket)),
      _req(std::move(req)) {}

void UserSession::process_request() {
  response_.version(_req.version());
  response_.keep_alive(false);

  switch (_req.method()) {
    case http::verb::get:
    case http::verb::post:
      response_.result(http::status::ok);
      response_.set(http::field::server, "dglux_server");
      create_response();
      break;

    default:
      response_.result(http::status::bad_request);
      response_.set(http::field::content_type, "text/plain");
      boost::beast::ostream(response_.body())
          << "Invalid request-method '" << _req.method_string().to_string()
          << "'";
      break;
  }

  write_response();
}

void UserSession::create_response() {
  if (_req.target() == "/") {
    response_.set(http::field::content_type, "text/html");
    response_.set(http::field::location, "/login");
    response_.result(http::status::found);
    boost::beast::ostream(response_.body())
        << "<html>\n"
        << "<head><title>Root</title></head>\n"
        << "<body>\n"
        << "<h1>Redirection</h1>\n"
        << "<p>Redirecting to login...</p>\n"
        << "</body>\n"
        << "</html>\n";
    file_flag = false;
  } else if (_req.target() == "/login" && _req.method() == http::verb::get) {
    boost::beast::string_view content_type = _req[http::field::content_type];
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open("/home/yg/Downloads/dsa/dglux-server/www/login.html",
              boost::beast::file_mode::scan, ec);
    auto const size = body.size();
    resp_ = {std::piecewise_construct, std::make_tuple(std::move(body)),
             std::make_tuple(http::status::ok, _req.version())};
    resp_.set(http::field::content_type, "text/html");
    resp_.content_length(size);
    resp_.keep_alive(_req.keep_alive());
    file_flag = true;
  } else if (_req.target() == "/login" && _req.method() == http::verb::post) {
    boost::beast::string_view content_type = _req[http::field::content_type];
    auto kvMap = Parse(_req.body());
    if (kvMap.at("username") == "dgSuper" &&
        base64_decode(kvMap.at("password")) == "dglux1234") {
      std::cout << "User logged in successfully" << std::endl;
      response_.set(http::field::content_type, "text/html");
      response_.result(http::status::found);
      boost::beast::ostream(response_.body())
          << "<html>\n"
          << "<head><title>Login</title></head>\n"
          << "<body>\n"
          << "<h1>Login state:</h1>\n"
          << "<p>User logged in successfully</p>\n"
          << "</body>\n"
          << "</html>\n";
    }
    file_flag = false;
  } else {
    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "text/plain");
    boost::beast::ostream(response_.body()) << "File not found\r\n";
  }
}

void UserSession::write_response() {
  auto self = shared_from_this();
  if (file_flag == false) {
    response_.set(http::field::content_length, response_.body().size());

    http::async_write(_socket, response_, [self](boost::beast::error_code ec,
                                                 std::size_t) {
      self->_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      //        self->deadline_.cancel();
    });
  } else {
    http::async_write(_socket, resp_, [self](boost::beast::error_code ec,
                                             std::size_t) {
      self->_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      //        self->deadline_.cancel();
    });
  }
}
}