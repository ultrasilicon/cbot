#include "WebSocket.hpp"
#include "Utils.hpp"
#include <boost/asio/connect.hpp>
#include <boost/beast/core.hpp>
#include <functional>

typedef boost::system::error_code error_code;

WebSocket::WebSocket(asio::io_context &ioc, ssl::context &ssl_ctx)
    : ioc_(ioc),
      ssl_ctx_(ssl_ctx),
      resolver_(ioc),
      ws_(ioc, ssl_ctx)
{
}

void WebSocket::connect(const std::string &host, const std::string &port, const std::string &target, ConnectCallback connect_callback)
{
    host_ = host;
    target_ = target;
    connect_callback_ = connect_callback;
    // Set SNI for SSL handshake.
    SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str());
    resolver_.async_resolve(host, port, cbot::bind(&WebSocket::on_dns_resolve, this));
}

void WebSocket::write(const std::string &message, WriteCallback write_callback)
{
    ws_.async_write(asio::buffer(message), write_callback);
}

void WebSocket::read(ReadCallback read_callback)
{
    ws_.async_read(buffer_,
                   [this, read_callback](error_code ec, std::size_t bytes_transferred)
                   {
                       std::string data = beast::buffers_to_string(buffer_.data());
                       buffer_.consume(buffer_.size());
                       read_callback(ec, bytes_transferred, data);
                   });
}

void WebSocket::on_dns_resolve(error_code ec, tcp::resolver::results_type results)
{
    if (ec)
    {
        connect_callback_(ec);
        return;
    }
    asio::async_connect(
        ws_.next_layer().next_layer(),
        results.begin(),
        results.end(),
        std::bind(&WebSocket::on_connect, this, std::placeholders::_1, std::placeholders::_2));
}

void WebSocket::on_connect(error_code ec, tcp::resolver::results_type::iterator)
{
    if (ec)
    {
        connect_callback_(ec);
        return;
    }
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        cbot::bind(&WebSocket::on_ssl_handshake, this));
}

void WebSocket::on_ssl_handshake(error_code ec)
{
    if (ec)
    {
        connect_callback_(ec);
        return;
    }
    ws_.async_handshake(host_, target_,
                        cbot::bind(&WebSocket::on_ws_handshake, this));
}

void WebSocket::on_ws_handshake(error_code ec)
{
    if (connect_callback_)
        connect_callback_(ec);
}
