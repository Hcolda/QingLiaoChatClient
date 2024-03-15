#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <QThread>
#include <QString>

#include <string>
#include <asio.hpp>

namespace qls
{
    using ReceiveStdStringFunction = std::function<void(std::string)>;
    using ReceiveQStringFunction = std::function<void(QString)>;

    inline std::string socket2ip(const asio::ip::tcp::socket& s);
    inline std::string showBinaryData(const std::string& data);

    class BaseNetwork
    {
    public:
        BaseNetwork() = default;
        virtual ~BaseNetwork() = default;

        virtual void connect() {}
        virtual void disconnect() {};

        virtual void send_data(const std::string&) {};
        virtual void send_data(const QString& data) { BaseNetwork::send_data(data.toStdString()); }

        virtual bool add_received_stdstring_callback(const std::string&, ReceiveStdStringFunction) { return false; }
        virtual bool add_received_qstring_callback(const std::string&, ReceiveQStringFunction) { return false; }
        virtual bool remove_received_stdstring_callback(const std::string&) { return false; }
        virtual bool remove_received_qstring_callback(const std::string&) { return false; }

        virtual bool add_connected_callback(const std::string&, std::function<void()>) { return false; }
        virtual bool add_disconnected_callback(const std::string&, std::function<void()>) { return false; }
        virtual bool remove_connected_callback(const std::string&) { return false; }
        virtual bool remove_disconnected_callback(const std::string&) { return false; }
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
        virtual bool add_received_qstring_callback(const std::string&, ReceiveQStringFunction);
        virtual bool remove_received_stdstring_callback(const std::string&);
        virtual bool remove_received_qstring_callback(const std::string&);

        virtual bool add_connected_callback(const std::string&, std::function<void()>);
        virtual bool add_disconnected_callback(const std::string&, std::function<void()>);
        virtual bool remove_connected_callback(const std::string&);
        virtual bool remove_disconnected_callback(const std::string&);

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

    struct ClientNetworkImpl;

    class ClientNetwork : public BaseNetwork
    {
    public:
        ClientNetwork(BaseNetwork&);
        ~ClientNetwork() = default;

        virtual bool add_received_stdstring_callback(const std::string&, ReceiveStdStringFunction);
        virtual bool add_received_qstring_callback(const std::string&, ReceiveQStringFunction);
        virtual bool remove_received_stdstring_callback(const std::string&);
        virtual bool remove_received_qstring_callback(const std::string&);

        virtual bool add_connected_callback(const std::string&, std::function<void()>);
        virtual bool add_disconnected_callback(const std::string&, std::function<void()>);
        virtual bool remove_connected_callback(const std::string&);
        virtual bool remove_disconnected_callback(const std::string&);

    private:
        std::shared_ptr<ClientNetworkImpl>  network_impl_;
        BaseNetwork&                        baseNetwork_;
    };
}

#endif // !NETWORK_HPP
