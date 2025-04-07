#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <functional>
#include <string>

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class WebSocket {
public:
    using ConnectCallback = std::function<void(boost::system::error_code)>;
    using WriteCallback = std::function<void(boost::system::error_code, std::size_t)>;
    using ReadCallback = std::function<void(boost::system::error_code, std::size_t, const std::string&)>;

    WebSocket(asio::io_context &ioc, ssl::context &ssl_ctx);

    void connect(const std::string &host, const std::string &port, const std::string &target, ConnectCallback connect_callback);
    void write(const std::string &message, WriteCallback write_callback);
    void read(ReadCallback read_callback);

private:
    asio::io_context &ioc_;
    ssl::context &ssl_ctx_;
    tcp::resolver resolver_;
    websocket::stream<ssl::stream<tcp::socket>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string target_;
    ConnectCallback connect_callback_;

    void on_dns_resolve(boost::system::error_code ec, tcp::resolver::results_type results);
    void on_connect(boost::system::error_code ec, tcp::resolver::results_type::iterator _);
    void on_ssl_handshake(boost::system::error_code ec);
    void on_ws_handshake(boost::system::error_code ec);
};

#endif // WEBSOCKET_HPP
