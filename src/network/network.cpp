#include "network.h"

#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>
#include <unordered_map>

#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <Json.h>

#include "definition.hpp"
#include "websiteFunctions.hpp"
#include "package.h"
#include "dataPackage.h"

std::string socket2ip(const asio::ip::tcp::socket& s)
{
    auto ep = s.remote_endpoint();
    return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
}

std::string showBinaryData(const std::string& data)
{
    auto isShowableCharactor = [](unsigned char ch) -> bool {
        return 32 <= ch && ch <= 126;
        };

    std::string result;

    for (const auto& i : data)
    {
        if (isShowableCharactor(static_cast<unsigned char>(i)))
        {
            result += i;
        }
        else
        {
            std::string hex;
            int locch = static_cast<unsigned char>(i);
            while (locch)
            {
                if (locch % 16 < 10)
                {
                    hex += ('0' + (locch % 16));
                    locch /= 16;
                    continue;
                }
                switch (locch % 16)
                {
                case 10:
                    hex += 'a';
                    break;
                case 11:
                    hex += 'b';
                    break;
                case 12:
                    hex += 'c';
                    break;
                case 13:
                    hex += 'd';
                    break;
                case 14:
                    hex += 'e';
                    break;
                case 15:
                    hex += 'f';
                    break;
                }
                locch /= 16;
            }

            //result += "\\x" + (hex.size() == 1 ? "0" + hex : hex);
            if (hex.empty())
            {
                result += "\\x00";
            }
            else if (hex.size() == 1)
            {
                result += "\\x0" + hex;
            }
            else
            {
                result += "\\x" + hex;
            }
        }
    }

    return result;
}

namespace qls
{
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;
    using namespace std::placeholders;
    using asio::ip::tcp;

    struct NetworkImpl
    {
        std::string host = "localhost";
        int         port = 55555;

        asio::io_context                        io_context;
        std::shared_ptr<asio::ip::tcp::socket>  socket_ptr;
        std::atomic<bool>                       is_running;
        std::atomic<bool>                       is_receiving;
        std::mutex                              mutex;
        std::condition_variable                 condition_variable;
        Package                                 package;
        asio::ip::tcp::resolver::results_type   endpoints;
        std::string                             input_buffer;

        asio::steady_timer                      deadline_timer;
        asio::steady_timer                      heartbeat_timer;

        std::unordered_map<std::string,
            ReceiveStdStringFunction>   revceiveStdStringFunction_map;
        std::shared_mutex               revceiveStdStringFunction_map_mutex;
        std::unordered_map<std::string,
            ReceiveQStringFunction>     receiveQStringFunction_map;
        std::shared_mutex               receiveQStringFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      connectedCallbackFunction_map;
        std::shared_mutex               connectedCallbackFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      disconnectedCallbackFunction_map;
        std::shared_mutex               disconnectedCallbackFunction_map_mutex;

        NetworkImpl() :
            deadline_timer(io_context),
            heartbeat_timer(io_context) {}
        ~NetworkImpl() = default;
    };

    Network::Network() :
        network_impl_(std::make_shared<NetworkImpl>())
    {
        network_impl_->is_running = true;
        network_impl_->is_receiving = false;
        QThread::start();
    }

    Network::~Network()
    {
        network_impl_->is_running = false;
        network_impl_->io_context.stop();
        network_impl_->condition_variable.notify_all();
        QThread::wait();
    }

    void Network::connect()
    {
        asio::io_service io_service;
        asio::ip::tcp::resolver resolver(io_service);

        asio::ip::tcp::resolver::query resolver_query(network_impl_->host,
            std::to_string(network_impl_->port));
        std::error_code ec;

        asio::ip::tcp::resolver::iterator it =
            resolver.resolve(resolver_query, ec);

        asio::ip::tcp::resolver::iterator it_end;

        if (it == it_end)
            throw std::runtime_error("there is not a service match the host");

        {
            network_impl_->is_receiving = false;
            std::unique_lock<std::mutex> lock(network_impl_->mutex);
            std::error_code ignored_error;
            network_impl_->socket_ptr->close(ignored_error);
            network_impl_->endpoints = resolver.resolve(resolver_query, ec);
        }

        network_impl_->condition_variable.notify_all();
    }

    void Network::disconnect()
    {
        network_impl_->is_receiving = false;
        {
            std::unique_lock<std::mutex> lock(network_impl_->mutex);
            std::error_code ignored_error;
            network_impl_->socket_ptr->close(ignored_error);
            network_impl_->deadline_timer.cancel();
            network_impl_->heartbeat_timer.cancel();
        }
        network_impl_->condition_variable.notify_all();
    }

    void Network::send_data(const std::string& data)
    {
        if (!network_impl_->is_receiving)
            throw std::runtime_error("socket is not able to use");

        // network_impl_->socket_ptr->async_send(asio::buffer(data), [](auto, auto){return;});
        asio::async_write(*(network_impl_->socket_ptr),
            asio::buffer(data), [](auto, auto){return;});
    }

    bool Network::add_received_stdstring_callback(const std::string& name, ReceiveStdStringFunction func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->revceiveStdStringFunction_map_mutex);

        auto iter = network_impl_->revceiveStdStringFunction_map.find(name);
        if (iter != network_impl_->revceiveStdStringFunction_map.end())
            return false;

        network_impl_->revceiveStdStringFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::add_received_qstring_callback(const std::string& name, ReceiveQStringFunction func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->receiveQStringFunction_map_mutex);

        auto iter = network_impl_->receiveQStringFunction_map.find(name);
        if (iter != network_impl_->receiveQStringFunction_map.end())
            return false;

        network_impl_->receiveQStringFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_received_stdstring_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->revceiveStdStringFunction_map_mutex);

        auto iter = network_impl_->revceiveStdStringFunction_map.find(name);
        if (iter == network_impl_->revceiveStdStringFunction_map.end())
            return false;

        network_impl_->revceiveStdStringFunction_map.erase(iter);
        return true;
    }

    bool Network::remove_received_qstring_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->receiveQStringFunction_map_mutex);

        auto iter = network_impl_->receiveQStringFunction_map.find(name);
        if (iter == network_impl_->receiveQStringFunction_map.end())
            return false;

        network_impl_->receiveQStringFunction_map.erase(iter);
        return true;
    }

    bool Network::add_connected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->connectedCallbackFunction_map_mutex);

        auto iter = network_impl_->connectedCallbackFunction_map.find(name);
        if (iter != network_impl_->connectedCallbackFunction_map.end())
            return false;

        network_impl_->connectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::add_disconnected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->disconnectedCallbackFunction_map_mutex);

        auto iter = network_impl_->disconnectedCallbackFunction_map.find(name);
        if (iter != network_impl_->disconnectedCallbackFunction_map.end())
            return false;

        network_impl_->disconnectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_connected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->connectedCallbackFunction_map_mutex);

        auto iter = network_impl_->connectedCallbackFunction_map.find(name);
        if (iter == network_impl_->connectedCallbackFunction_map.end())
            return false;

        network_impl_->connectedCallbackFunction_map.erase(iter);
        return true;
    }

    bool Network::remove_disconnected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->disconnectedCallbackFunction_map_mutex);

        auto iter = network_impl_->disconnectedCallbackFunction_map.find(name);
        if (iter == network_impl_->disconnectedCallbackFunction_map.end())
            return false;

        network_impl_->disconnectedCallbackFunction_map.erase(iter);
        return true;
    }

    void Network::run()
    {
        while (network_impl_->is_running)
        {
            std::unique_lock<std::mutex> lock(network_impl_->mutex);
            network_impl_->condition_variable.wait(lock,
                [&]() {return !network_impl_->is_running || !network_impl_->is_receiving; });

            if (!network_impl_->is_running)
                return;

            try
            {
                lock.unlock();
                start_connect(network_impl_->endpoints.begin());
                network_impl_->io_context.run();
            }
            catch (...) {}
        }
    }

    void Network::start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running)
            return;

        if (endpoint_iter != network_impl_->endpoints.end())
        {
            network_impl_->deadline_timer.expires_after(std::chrono::seconds(60));

            network_impl_->socket_ptr->async_connect(endpoint_iter->endpoint(),
                std::bind(&Network::handle_connect,
                    this, _1, endpoint_iter));
        }
        else
        {
            network_impl_->is_receiving = false;
        }
    }

    void Network::handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running)
            return;

        if (!network_impl_->socket_ptr->is_open())
        {
            start_connect(++endpoint_iter);
        }
        else if (error)
        {
            network_impl_->socket_ptr->close();
            start_connect(++endpoint_iter);
        }
        else
        {
            network_impl_->is_receiving = true;
            async_read();
        }
    }

    void Network::async_read()
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running || !network_impl_->is_receiving)
            return;

        network_impl_->deadline_timer.expires_after(std::chrono::seconds(30));

        network_impl_->socket_ptr->async_read_some(asio::buffer(network_impl_->input_buffer),
            std::bind(&Network::handle_read, this, _1, _2));
    }

    void Network::handle_read(const std::error_code& error, std::size_t n)
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running || !network_impl_->is_receiving)
            return;

        if (!error)
        {
            network_impl_->package.write(network_impl_->input_buffer);
            network_impl_->input_buffer.clear();

            if (network_impl_->package.canRead())
            {
                emit received_message(std::move(QString::fromStdString(
                                        network_impl_->package.read())));
            }

            async_read();
        }
        else
        {
            network_impl_->is_receiving = false;
            std::error_code ignored_error;
            network_impl_->socket_ptr->close(ignored_error);
            network_impl_->deadline_timer.cancel();
            network_impl_->heartbeat_timer.cancel();
        }
    }

    void Network::heart_beat_write()
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running || !network_impl_->is_receiving)
            return;

        auto pack = DataPackage::makePackage("heartbeat");
        pack->type = 4;

        asio::async_write(*(network_impl_->socket_ptr), asio::buffer(pack->packageToString(), 1),
            std::bind(&Network::handle_heart_beat_write, this, _1));
    }

    void Network::handle_heart_beat_write(const std::error_code& error)
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running || !network_impl_->is_receiving)
            return;

        if (!error)
        {
            network_impl_->heartbeat_timer.expires_after(std::chrono::seconds(10));
            network_impl_->heartbeat_timer.async_wait(std::bind(&Network::heart_beat_write, this));
        }
        else
        {
            network_impl_->is_receiving = false;
            std::error_code ignored_error;
            network_impl_->socket_ptr->close(ignored_error);
            network_impl_->deadline_timer.cancel();
            network_impl_->heartbeat_timer.cancel();
        }
    }

    void Network::check_deadline()
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        if (!network_impl_->is_running || !network_impl_->is_receiving)
            return;

        if (network_impl_->deadline_timer.expiry() <= asio::steady_timer::clock_type::now())
        {
            network_impl_->socket_ptr->close();
            network_impl_->is_receiving = false;

            network_impl_->deadline_timer.expires_at(asio::steady_timer::time_point::max());

            start_connect(network_impl_->endpoints.begin());
        }

        network_impl_->deadline_timer.async_wait(std::bind(&Network::check_deadline, this));
    }

    struct ClientNetworkImpl
    {
        std::unordered_map<std::string,
            ReceiveStdStringFunction>   revceiveStdStringFunction_map;
        std::shared_mutex               revceiveStdStringFunction_map_mutex;
        std::unordered_map<std::string,
            ReceiveQStringFunction>     receiveQStringFunction_map;
        std::shared_mutex               receiveQStringFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      connectedCallbackFunction_map;
        std::shared_mutex               connectedCallbackFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      disconnectedCallbackFunction_map;
        std::shared_mutex               disconnectedCallbackFunction_map_mutex;
    };

    ClientNetwork::ClientNetwork(BaseNetwork& network) :
        baseNetwork_(network),
        network_impl_(std::make_shared<ClientNetworkImpl>())
    {
        
    }

    bool ClientNetwork::add_received_stdstring_callback(const std::string& name, ReceiveStdStringFunction func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->revceiveStdStringFunction_map_mutex);

        auto iter = network_impl_->revceiveStdStringFunction_map.find(name);
        if (iter != network_impl_->revceiveStdStringFunction_map.end())
            return false;

        network_impl_->revceiveStdStringFunction_map[name] = std::move(func);
        return true;
    }

    bool ClientNetwork::add_received_qstring_callback(const std::string& name, ReceiveQStringFunction func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->receiveQStringFunction_map_mutex);

        auto iter = network_impl_->receiveQStringFunction_map.find(name);
        if (iter != network_impl_->receiveQStringFunction_map.end())
            return false;

        network_impl_->receiveQStringFunction_map[name] = std::move(func);
        return true;
    }

    bool ClientNetwork::remove_received_stdstring_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->revceiveStdStringFunction_map_mutex);

        auto iter = network_impl_->revceiveStdStringFunction_map.find(name);
        if (iter == network_impl_->revceiveStdStringFunction_map.end())
            return false;

        network_impl_->revceiveStdStringFunction_map.erase(iter);
        return true;
    }

    bool ClientNetwork::remove_received_qstring_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->receiveQStringFunction_map_mutex);

        auto iter = network_impl_->receiveQStringFunction_map.find(name);
        if (iter == network_impl_->receiveQStringFunction_map.end())
            return false;

        network_impl_->receiveQStringFunction_map.erase(iter);
        return true;
    }

    bool ClientNetwork::add_connected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->connectedCallbackFunction_map_mutex);

        auto iter = network_impl_->connectedCallbackFunction_map.find(name);
        if (iter != network_impl_->connectedCallbackFunction_map.end())
            return false;

        network_impl_->connectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool ClientNetwork::add_disconnected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->disconnectedCallbackFunction_map_mutex);

        auto iter = network_impl_->disconnectedCallbackFunction_map.find(name);
        if (iter != network_impl_->disconnectedCallbackFunction_map.end())
            return false;

        network_impl_->disconnectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool ClientNetwork::remove_connected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->connectedCallbackFunction_map_mutex);

        auto iter = network_impl_->connectedCallbackFunction_map.find(name);
        if (iter == network_impl_->connectedCallbackFunction_map.end())
            return false;

        network_impl_->connectedCallbackFunction_map.erase(iter);
        return true;
    }

    bool ClientNetwork::remove_disconnected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            network_impl_->disconnectedCallbackFunction_map_mutex);

        auto iter = network_impl_->disconnectedCallbackFunction_map.find(name);
        if (iter == network_impl_->disconnectedCallbackFunction_map.end())
            return false;

        network_impl_->disconnectedCallbackFunction_map.erase(iter);
        return true;
    }
}