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

/**
 * @class WebSocket
 * @brief Manages WebSocket connections, including DNS resolution, SSL handshake, and message handling.
 */
class WebSocket {
public:
    using ConnectCallback = std::function<void(boost::system::error_code)>;
    using WriteCallback = std::function<void(boost::system::error_code, std::size_t)>;
    using ReadCallback = std::function<void(boost::system::error_code, std::size_t, const std::string&)>;

    /**
     * @brief Constructs a WebSocket object with the given io and SSL contexts.
     * @param ioc The io context for boost async
     * @param ssl_ctx The ssl context
     */
    // TODO: ssl::context doesn't need to be passed in but I don't have time to refactor
    WebSocket(asio::io_context &ioc, ssl::context &ssl_ctx);

    /**
     * @brief Initiates a wss connection to the specified host, port, and target.
     * @param host The server host.
     * @param port The server port.
     * @param target The WebSocket target endpoint.
     * @param connect_callback Callback to handle connection completion.
     */
    void connect(const std::string &host, const std::string &port, const std::string &target, ConnectCallback connect_callback);

    /**
     * @brief Sends a message over the WebSocket connection.
     * @param message The message to send.
     * @param write_callback Callback on write complete.
     */
    void write(const std::string &message, WriteCallback write_callback);

    /**
     * @brief Reads a message from the WebSocket connection.
     * @param read_callback Callback on read complete.
     */
    void read(ReadCallback read_callback);

private:
    std::string host_;
    std::string target_;
    ConnectCallback connect_callback_;

    // some boost networking specific contexts/buffers
    asio::io_context &ioc_;
    ssl::context &ssl_ctx_;
    tcp::resolver resolver_;
    websocket::stream<ssl::stream<tcp::socket>> ws_;
    beast::flat_buffer buffer_;

    void on_dns_resolve(boost::system::error_code ec, tcp::resolver::results_type results);
    void on_connect(boost::system::error_code ec, tcp::resolver::results_type::iterator _);
    void on_ssl_handshake(boost::system::error_code ec);
    void on_ws_handshake(boost::system::error_code ec);
};

#endif // WEBSOCKET_HPP
