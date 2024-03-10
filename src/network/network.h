#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <QThread>
#include <QString>

#include <string>
#include <asio.hpp>

namespace qls
{
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;

    /*
    * @brief 读取socket地址到string
    * @param socket
    * @return string socket的地址
    */
    inline std::string socket2ip(const asio::ip::tcp::socket& s);

    inline std::string showBinaryData(const std::string& data);

    struct NetworkImpl;

    class Network : public QThread
    {
        Q_OBJECT

    public:
        Network();
        ~Network();

        void connect();
        void disconnect();

        void send_data(const std::string& data);
        void send_data(const QString& data);

    signals:
        void disconnected();
        void connected();
        void received_message(QString);

    protected:
        void run();

        void start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
        void handle_connect(const std::error_code& error,
            asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
        void async_read();
        void handle_read(const std::error_code& error, std::size_t n);
        void heart_beat_write();
        void handle_heart_beat_write(const std::error_code& error);
        void check_deadline();

    private:
        std::shared_ptr<NetworkImpl> network_impl_;
    };
}

#endif // !NETWORK_HPP
