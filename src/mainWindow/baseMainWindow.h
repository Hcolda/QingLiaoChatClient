#ifndef BASE_MAIN_WINDOW_H
#define BASE_MAIN_WINDOW_H

#include <functional>
#include <string>
#include <system_error>

namespace qls
{
    using SendPrivateRoomMessageFunc = std::function<void(long long, const std::string&)>;
    using SendGroupRoomMessageFunc = std::function<void(long long, const std::string&)>;
    using SendCommonMessageFunc = std::function<void(const std::string&)>;

    enum class MessageType
    {
        DEFAULT_MESSAGE = 0,
        TIP_MESSAGE
    };

    class BaseMainWindow
    {
    public:
        BaseMainWindow() = default;
        virtual ~BaseMainWindow() = default;

        virtual bool addPrivateRoom(long long user_id) { return false; }
        virtual bool romovePrivateRoom(long long user_id) { return false; }
        virtual bool addGroupRoom(long long roon_id) { return false; }
        virtual bool removeGroupRoom(long long roon_id) { return false; }

        virtual void addPrivateRoomMessage(long long user_id, MessageType type, const std::string& message) {}
        virtual bool removePrivateRoomMessage(size_t index) { return false; }
        virtual void addGroupRoomMessage(long long group_id, long long sender_id, MessageType type, const std::string& message) {}
        virtual bool removeGroupRoomMessage(size_t index) { return false; }

        virtual void setPrivateRoomMessageCallback(SendPrivateRoomMessageFunc func) {}
        virtual void setGroupRoomMessageCallback(SendGroupRoomMessageFunc func) {}
        virtual void setCommonMessageCallback(SendCommonMessageFunc func) {}

        virtual void connected_callback() {}
        virtual void disconnected_callback() {}
        virtual void connected_error_callback(std::error_code) {}
    };
}

#endif // !BASE_MAIN_WINDOW_H
