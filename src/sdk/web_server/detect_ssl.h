#ifndef DSA_SDK_DETECT_SSL_H
#define DSA_SDK_DETECT_SSL_H

#include <boost/assert.hpp>
#include <boost/beast/core.hpp>
#include <boost/config.hpp>
#include <boost/logic/tribool.hpp>

namespace dsa {

template <class ConstBufferSequence>
boost::tribool is_ssl_handshake(ConstBufferSequence const& buffers);

template <class ConstBufferSequence>
boost::tribool is_ssl_handshake(ConstBufferSequence const& buffers) {
  static_assert(
      boost::asio::is_const_buffer_sequence<ConstBufferSequence>::value,
      "ConstBufferSequence requirements not met");

  if (boost::asio::buffer_size(buffers) < 1) return boost::indeterminate;

  unsigned char v;
  boost::asio::buffer_copy(boost::asio::buffer(&v, 1), buffers);

  if (v != 0x16) {
    return false;
  }

  if (boost::asio::buffer_size(buffers) < 4) return boost::indeterminate;

  return true;
}

template <class SyncReadStream, class DynamicBuffer>
boost::tribool detect_ssl(SyncReadStream& stream, DynamicBuffer& buffer,
                          boost::beast::error_code& ec) {
  namespace beast = boost::beast;

  static_assert(beast::is_sync_read_stream<SyncReadStream>::value,
                "SyncReadStream requirements not met");
  static_assert(boost::asio::is_dynamic_buffer<DynamicBuffer>::value,
                "DynamicBuffer requirements not met");

  for (;;) {
    auto const result = is_ssl_handshake(buffer.data());

    if (!boost::indeterminate(result)) {
      ec.assign(0, ec.category());
      return result;
    }

    BOOST_ASSERT(buffer.size() < 4);

    auto const mutable_buffer = buffer.prepare(beast::read_size(buffer, 1536));

    std::size_t const bytes_transferred = stream.read_some(mutable_buffer, ec);

    if (ec) break;

    buffer.commit(bytes_transferred);
  }

  return false;
}

template <class AsyncReadStream, class DynamicBuffer, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(/*< `BOOST_ASIO_INITFN_RESULT_TYPE` customizes the
                                 return value based on the completion token >*/
                              CompletionToken,
                              void(boost::beast::error_code,
                                   boost::tribool)) /*< This is the signature
                                                       for the completion
                                                       handler >*/
async_detect_ssl(AsyncReadStream& stream, DynamicBuffer& buffer,
                 CompletionToken&& token);

template <class AsyncReadStream, class DynamicBuffer, class Handler>
class detect_ssl_op;

template <class AsyncReadStream, class DynamicBuffer, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                              void(boost::beast::error_code, boost::tribool))
async_detect_ssl(AsyncReadStream& stream, DynamicBuffer& buffer,
                 CompletionToken&& token) {
  namespace beast = boost::beast;

  static_assert(beast::is_async_read_stream<AsyncReadStream>::value,
                "SyncReadStream requirements not met");
  static_assert(boost::asio::is_dynamic_buffer<DynamicBuffer>::value,
                "DynamicBuffer requirements not met");

  boost::asio::async_completion<CompletionToken,
                                void(beast::error_code, boost::tribool)>
      init{token};

  detect_ssl_op<AsyncReadStream, DynamicBuffer,
                BOOST_ASIO_HANDLER_TYPE(
                    CompletionToken, void(beast::error_code, boost::tribool))>{
      stream, buffer, init.completion_handler}(beast::error_code{}, 0);

  return init.result.get();
}

template <class AsyncReadStream, class DynamicBuffer, class Handler>
class detect_ssl_op {
  int step_ = 0;

  AsyncReadStream& stream_;
  DynamicBuffer& buffer_;
  Handler handler_;
  boost::tribool result_ = false;

 public:
  detect_ssl_op(detect_ssl_op const&) = default;

  template <class DeducedHandler>
  detect_ssl_op(AsyncReadStream& stream, DynamicBuffer& buffer,
                DeducedHandler&& handler)
      : stream_(stream),
        buffer_(buffer),
        handler_(std::forward<DeducedHandler>(handler)) {}

  using allocator_type = boost::asio::associated_allocator_t<Handler>;

  allocator_type get_allocator() const noexcept {
    return boost::asio::get_associated_allocator(handler_);
  }

  using executor_type =
      boost::asio::associated_executor_t<Handler,
                                         decltype(stream_.get_executor())>;

  executor_type get_executor() const noexcept {
    return boost::asio::get_associated_executor(handler_,
                                                stream_.get_executor());
  }

  friend bool asio_handler_is_continuation(detect_ssl_op* op) {
    using boost::asio::asio_handler_is_continuation;

    return op->step_ > 2 ||
           asio_handler_is_continuation(std::addressof(op->handler_));
  }

  void operator()(boost::beast::error_code ec, std::size_t bytes_transferred);
};

template <class AsyncStream, class DynamicBuffer, class Handler>
void detect_ssl_op<AsyncStream, DynamicBuffer, Handler>::operator()(
    boost::beast::error_code ec, std::size_t bytes_transferred) {
  namespace beast = boost::beast;

  switch (step_) {
    case 0:
      result_ = is_ssl_handshake(buffer_.data());

      if (!boost::indeterminate(result_)) {
        step_ = 1;
        return boost::asio::post(stream_.get_executor(),
                                 beast::bind_handler(std::move(*this), ec, 0));
      }

      BOOST_ASSERT(buffer_.size() < 4);

      step_ = 2;

    do_read:
      return stream_.async_read_some(
          buffer_.prepare(beast::read_size(buffer_, 1536)), std::move(*this));

    case 1:
      break;

    case 2:
      step_ = 3;
      BOOST_FALLTHROUGH;

    case 3:
      if (ec) {
        result_ = false;

        break;
      }

      buffer_.commit(bytes_transferred);

      result_ = is_ssl_handshake(buffer_.data());

      if (!boost::indeterminate(result_)) {
        break;
      }

      goto do_read;
  }

  handler_(ec, result_);
}
}
#endif  // DSA_SDK_DETECT_SSL_H
