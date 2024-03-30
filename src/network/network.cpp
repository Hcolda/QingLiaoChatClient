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

namespace qingliao
{
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::use_awaitable;
    namespace this_coro = asio::this_coro;
    using namespace std::placeholders;
    using asio::ip::tcp;

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
            std::function<void()>>      connectedCallbackFunction_map;
        std::shared_mutex               connectedCallbackFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void()>>      disconnectedCallbackFunction_map;
        std::shared_mutex               disconnectedCallbackFunction_map_mutex;
        std::unordered_map<std::string,
            std::function<void(std::error_code)>>
                                        connectedErrorCallbackFunction_map;
        std::shared_mutex               connectedErrorCallbackFunction_map_mutex;

        NetworkImpl() :
            deadline_timer(io_context),
            heartbeat_timer(io_context),
            socket_ptr(std::make_shared<asio::ip::tcp::socket>(io_context)) {}
        ~NetworkImpl() = default;
    };

    Network::Network() :
        m_network_impl(std::make_shared<NetworkImpl>())
    {
        m_network_impl->is_running = true;
        m_network_impl->is_receiving = false;
        QThread::start();
    }

    Network::~Network()
    {
        m_network_impl->is_running = false;
        m_network_impl->io_context.stop();
        m_network_impl->condition_variable.notify_all();
        QThread::wait();
    }

    void Network::connect()
    {
        asio::io_service io_service;
        asio::ip::tcp::resolver resolver(io_service);

        asio::ip::tcp::resolver::query resolver_query(m_network_impl->host,
            std::to_string(m_network_impl->port));
        std::error_code ec;

        asio::ip::tcp::resolver::iterator it =
            resolver.resolve(resolver_query, ec);

        asio::ip::tcp::resolver::iterator it_end;

        if (it == it_end)
            throw std::runtime_error("there is not a service match the host");

        {
            m_network_impl->is_receiving = false;
            std::unique_lock<std::mutex> lock(m_network_impl->mutex);
            m_network_impl->endpoints = resolver.resolve(resolver_query, ec);
        }

        m_network_impl->condition_variable.notify_all();
    }

    void Network::disconnect()
    {
        m_network_impl->is_receiving = false;
        {
            std::unique_lock<std::mutex> lock(m_network_impl->mutex);
            std::error_code ignored_error;
            m_network_impl->socket_ptr->close(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
        }
        m_network_impl->condition_variable.notify_all();
    }

    void Network::send_data(const std::string& data)
    {
        if (!m_network_impl->is_receiving)
            throw std::runtime_error("socket is not able to use");

        auto wrapper = std::make_shared<StringWrapper>(data);

        // m_network_impl->socket_ptr->async_send(asio::buffer(data), [](auto, auto){return;});
        asio::async_write(*(m_network_impl->socket_ptr),
            asio::buffer(wrapper->data), std::bind(&Network::handle_write, this, _1, _2, wrapper));
    }

    bool Network::add_received_stdstring_callback(const std::string& name, ReceiveStdStringFunction func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->revceiveStdStringFunction_map_mutex);

        auto iter = m_network_impl->revceiveStdStringFunction_map.find(name);
        if (iter != m_network_impl->revceiveStdStringFunction_map.end())
            return false;

        m_network_impl->revceiveStdStringFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_received_stdstring_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->revceiveStdStringFunction_map_mutex);

        auto iter = m_network_impl->revceiveStdStringFunction_map.find(name);
        if (iter == m_network_impl->revceiveStdStringFunction_map.end())
            return false;

        m_network_impl->revceiveStdStringFunction_map.erase(iter);
        return true;
    }

    bool Network::add_connected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedCallbackFunction_map_mutex);

        auto iter = m_network_impl->connectedCallbackFunction_map.find(name);
        if (iter != m_network_impl->connectedCallbackFunction_map.end())
            return false;

        m_network_impl->connectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_connected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedCallbackFunction_map_mutex);

        auto iter = m_network_impl->connectedCallbackFunction_map.find(name);
        if (iter == m_network_impl->connectedCallbackFunction_map.end())
            return false;

        m_network_impl->connectedCallbackFunction_map.erase(iter);
        return true;
    }

    bool Network::add_disconnected_callback(const std::string& name, std::function<void()> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->disconnectedCallbackFunction_map_mutex);

        auto iter = m_network_impl->disconnectedCallbackFunction_map.find(name);
        if (iter != m_network_impl->disconnectedCallbackFunction_map.end())
            return false;

        m_network_impl->disconnectedCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_disconnected_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->disconnectedCallbackFunction_map_mutex);

        auto iter = m_network_impl->disconnectedCallbackFunction_map.find(name);
        if (iter == m_network_impl->disconnectedCallbackFunction_map.end())
            return false;

        m_network_impl->disconnectedCallbackFunction_map.erase(iter);
        return true;
    }

    bool Network::add_connected_error_callback(const std::string& name, std::function<void(std::error_code)> func)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedErrorCallbackFunction_map_mutex);

        auto iter = m_network_impl->connectedErrorCallbackFunction_map.find(name);
        if (iter != m_network_impl->connectedErrorCallbackFunction_map.end())
            return false;

        m_network_impl->connectedErrorCallbackFunction_map[name] = std::move(func);
        return true;
    }

    bool Network::remove_connected_error_callback(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(
            m_network_impl->connectedErrorCallbackFunction_map_mutex);

        auto iter = m_network_impl->connectedErrorCallbackFunction_map.find(name);
        if (iter == m_network_impl->connectedErrorCallbackFunction_map.end())
            return false;

        m_network_impl->connectedErrorCallbackFunction_map.erase(iter);
        return true;
    }

    void Network::call_connected()
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->connectedCallbackFunction_map_mutex);

        for (const auto& [_, func] : m_network_impl->connectedCallbackFunction_map)
        {
            func();
        }
    }

    void Network::call_disconnect()
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->disconnectedCallbackFunction_map_mutex);

        for (const auto& [_, func] : m_network_impl->disconnectedCallbackFunction_map)
        {
            func();
        }
    }

    void Network::call_connected_error()
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->connectedErrorCallbackFunction_map_mutex);

        auto error_code = std::make_error_code(std::errc::not_connected);
        for (const auto& [_, func] : m_network_impl->connectedErrorCallbackFunction_map)
        {
            func(error_code);
        }
    }

    void Network::call_received_stdstring(std::string data)
    {
        std::shared_lock<std::shared_mutex> lock(
            m_network_impl->revceiveStdStringFunction_map_mutex);

        for (const auto& [_, func] : m_network_impl->revceiveStdStringFunction_map)
        {
            func(data);
        }
    }

    void Network::run()
    {
        while (m_network_impl->is_running)
        {
            std::unique_lock<std::mutex> lock(m_network_impl->mutex);
            m_network_impl->condition_variable.wait(lock,
                [&]() { return !m_network_impl->is_running || !m_network_impl->endpoints.empty(); });

            if (!m_network_impl->is_running)
                return;

            try
            {
                lock.unlock();
                start_connect(m_network_impl->endpoints.begin());
                m_network_impl->io_context.run();
            }
            catch (...) {}
        }
    }

    void Network::start_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running)
            return;

        if (endpoint_iter != m_network_impl->endpoints.end())
        {
            m_network_impl->deadline_timer.expires_after(std::chrono::seconds(60));

            m_network_impl->socket_ptr->async_connect(endpoint_iter->endpoint(),
                std::bind(&Network::handle_connect,
                    this, _1, endpoint_iter));
        }
        else
        {
            m_network_impl->is_receiving = false;
            m_network_impl->endpoints = {};
            call_connected_error();
        }
    }

    void Network::handle_connect(const std::error_code& error, asio::ip::tcp::resolver::results_type::iterator endpoint_iter)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running)
            return;

        if (!m_network_impl->socket_ptr->is_open())
        {
            lock.unlock();
            start_connect(++endpoint_iter);
        }
        else if (error)
        {
            m_network_impl->socket_ptr->close();
            lock.unlock();
            start_connect(++endpoint_iter);
        }
        else
        {
            m_network_impl->is_receiving = true;
            lock.unlock();
            send_data(DataPackage::makePackage("test")->packageToString());
            async_read();
            heart_beat_write();
            call_connected();
        }
    }

    void Network::async_read()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;

        m_network_impl->deadline_timer.expires_after(std::chrono::seconds(30));

        m_network_impl->socket_ptr->async_read_some(asio::buffer(m_network_impl->input_buffer),
            std::bind(&Network::handle_read, this, _1, _2));
    }

    void Network::handle_read(const std::error_code& error, std::size_t n)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;

        if (!error)
        {
            m_network_impl->package.write(m_network_impl->input_buffer);
            m_network_impl->input_buffer.clear();

            if (m_network_impl->package.canRead())
            {
                call_received_stdstring(std::move(
                    m_network_impl->package.read()));
            }

            lock.unlock();
            async_read();
        }
        else
        {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->close(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
        }
    }

    void Network::heart_beat_write()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;

        auto pack = DataPackage::makePackage("heartbeat");
        pack->type = 4;

        auto wrapper = std::make_shared<StringWrapper>(pack->packageToString());

        asio::async_write(*(m_network_impl->socket_ptr), asio::buffer(wrapper->data),
            std::bind(&Network::handle_heart_beat_write, this, _1, wrapper));
    }

    void Network::handle_heart_beat_write(const std::error_code& error, std::shared_ptr<StringWrapper>)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;

        if (!error)
        {
            m_network_impl->heartbeat_timer.expires_after(std::chrono::seconds(10));
            m_network_impl->heartbeat_timer.async_wait(std::bind(&Network::heart_beat_write, this));
        }
        else
        {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->close(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
            call_disconnect();
        }
    }

    void Network::check_deadline()
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;

        if (m_network_impl->deadline_timer.expiry() <= asio::steady_timer::clock_type::now())
        {
            m_network_impl->socket_ptr->close();
            m_network_impl->is_receiving = false;

            m_network_impl->deadline_timer.expires_at(asio::steady_timer::time_point::max());
            call_disconnect();

            lock.unlock();
            start_connect(m_network_impl->endpoints.begin());
        }

        m_network_impl->deadline_timer.async_wait(std::bind(&Network::check_deadline, this));
    }

    void Network::handle_write(const std::error_code& error,
        std::size_t n,
        std::shared_ptr<StringWrapper>)
    {
        std::unique_lock<std::mutex> lock(m_network_impl->mutex);
        if (!m_network_impl->is_running || !m_network_impl->is_receiving)
            return;

        if (error)
        {
            m_network_impl->is_receiving = false;
            std::error_code ignored_error;
            m_network_impl->socket_ptr->close(ignored_error);
            m_network_impl->deadline_timer.cancel();
            m_network_impl->heartbeat_timer.cancel();
            call_disconnect();
        }
    }
}