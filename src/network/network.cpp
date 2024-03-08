#include "network.h"

#include <Logger.hpp>
#include <QuqiCrypto.hpp>
#include <Json.h>

#include "definition.hpp"
#include "websiteFunctions.hpp"

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
