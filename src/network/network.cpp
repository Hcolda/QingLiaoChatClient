#include "network.h"

#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <Json.h>

#include "definition.hpp"
#include "websiteFunctions.hpp"
#include "package.h"
#include "dataPackage.h"

std::string qls::socket2ip(const asio::ip::tcp::socket& s)
{
    auto ep = s.remote_endpoint();
    return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
}

std::string qls::showBinaryData(const std::string& data)
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

struct qls::NetworkImpl
{
    std::string host = "localhost";
    int         port = 55555;

    asio::io_context                        io_context;
    std::shared_ptr<asio::ip::tcp::socket>  socket_ptr;
    std::atomic<bool>                       is_running;
    std::atomic<bool>                       is_receiving;
    std::mutex                              mutex;
    std::condition_variable                 condition_variable;
    qls::Package                            package;
};

qls::Network::Network() :
    network_impl_(std::make_shared<NetworkImpl>())
{
    network_impl_->is_running = true;
    QThread::start();
}

qls::Network::~Network()
{
    network_impl_->is_running = false;
    network_impl_->condition_variable.notify_all();
    QThread::wait();
}

void qls::Network::connect()
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

        for (; it != it_end; ++it)
        {
            try
            {
                asio::ip::tcp::endpoint ep = it->endpoint();
                asio::ip::tcp::socket socket(network_impl_->io_context);
                socket.connect(ep);
                network_impl_->socket_ptr = std::make_shared<asio::ip::tcp::socket>(
                    std::move(socket));
                network_impl_->package.setBuffer({});
                break;
            }
            catch (...)
            {
                continue;
            }
        }
    }

    network_impl_->condition_variable.notify_all();
    emit connected();
}

void qls::Network::disconnect()
{
    network_impl_->is_receiving = false;
}

void qls::Network::run()
{
    while (network_impl_->is_running)
    {
        std::unique_lock<std::mutex> lock(network_impl_->mutex);
        network_impl_->condition_variable.wait(lock,
            [&](){return !network_impl_->is_running;});

        if (!network_impl_->is_running)
            return;
        if (!network_impl_->is_receiving)
            continue;

        while (network_impl_->is_running && network_impl_->is_receiving)
        {
            char buffer[8192]{ 0 };
            size_t size = 0;
            try
            {
                size = network_impl_->socket_ptr->read_some(asio::buffer(buffer));
            }
            catch (const std::exception&)
            {
                network_impl_->is_receiving = false;
                break;
            }
            
            network_impl_->package.write({buffer, size});
            if (network_impl_->package.canRead())
            {
                emit received_message(QString::fromStdString(
                    network_impl_->package.read()));
            }
        }
        emit disconnected();
    }
}
