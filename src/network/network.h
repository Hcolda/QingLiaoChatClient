#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <QThread>
#include <QString>

#include <string>
#include <asio.hpp>

namespace qls
{
    using ReceiveStdStringFunction = std::function<void(std::string)>;

    inline std::string socket2ip(const asio::ip::tcp::socket& s);
    inline std::string showBinaryData(const std::string& data);

    struct NetworkImpl;

    class BaseNetwork
    {
    public:
        BaseNetwork() = default;
        virtual ~BaseNetwork() = default;

        virtual void connect() {}
        virtual void disconnect() {}

        virtual void send_data(const std::string&) {}
        virtual void send_data(const QString& data) { send_data(data.toStdString()); }

        virtual bool add_received_stdstring_callback(const std::string&, ReceiveStdStringFunction) { return false; }
        virtual bool remove_received_stdstring_callback(const std::string&) { return false; }

        virtual bool add_connected_callback(const std::string&, std::function<void()>) { return false; }
        virtual bool remove_connected_callback(const std::string&) { return false; }
        virtual bool add_disconnected_callback(const std::string&, std::function<void()>) { return false; }
        virtual bool remove_disconnected_callback(const std::string&) { return false; }
        virtual bool add_connected_error_callback(const std::string&, std::function<void(std::error_code)>) { return false; }
        virtual bool remove_connected_error_callback(const std::string&) { return false; }
    };

    struct NetworkImpl;

    class Network : public QThread, public BaseNetwork
    {
        Q_OBJECT

    public:
        Network();
        ~Network();

        void connect();
        void disconnect();

        void send_data(const std::string& data);

        virtual bool add_received_stdstring_callback(const std::string&, ReceiveStdStringFunction);
        virtual bool remove_received_stdstring_callback(const std::string&);

        virtual bool add_connected_callback(const std::string&, std::function<void()>);
        virtual bool remove_connected_callback(const std::string&);
        virtual bool add_disconnected_callback(const std::string&, std::function<void()>);
        virtual bool remove_disconnected_callback(const std::string&);
        virtual bool add_connected_error_callback(const std::string&, std::function<void(std::error_code)>);
        virtual bool remove_connected_error_callback(const std::string&);

    protected:
        void call_connected();
        void call_disconnect();
        void call_connected_error();
        void call_received_stdstring(std::string);

        void run();

        void start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
        void handle_connect(const std::error_code& error,
            asio::ip::tcp::resolver::results_type::iterator endpoint_iter);
        void async_read();
        void handle_read(const std::error_code& error, std::size_t n);
        void heart_beat_write();
        void handle_heart_beat_write(const std::error_code& error);
        void check_deadline();
        void handle_write(const std::error_code& error, std::size_t n);

    private:
        std::shared_ptr<NetworkImpl> m_network_impl;
    };
}

#endif // !NETWORK_HPP
