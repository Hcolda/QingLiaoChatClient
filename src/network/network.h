#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

#include <QuqiCrypto.hpp>

#include "definition.hpp"
#include "package.h"
#include "dataPackage.h"

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

    class Network
    {
    public:
        Network() = default;
        ~Network() = default;
    };
}

#endif // !NETWORK_HPP
