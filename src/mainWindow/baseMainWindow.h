#ifndef BASE_MAIN_WINDOW_H
#define BASE_MAIN_WINDOW_H

#include <functional>
#include <string>

namespace qls
{
    using SendPrivateRoomMessage = std::function<void(long long, const std::string&)>;
    using SendGroupRoomMessage = std::function<void(long long, const std::string&)>;

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

        virtual void connected_callback() {}
        virtual void disconnected_callback() {}

        virtual bool addPrivateRoom(long long user_id) { return false; }
        virtual bool romovePrivateRoom(long long user_id) { return false; }
        virtual bool addGroupRoom(long long roon_id) { return false; }
        virtual bool removeGroupRoom(long long roon_id) { return false; }

        virtual void addPrivateRoomMessage(long long user_id, MessageType type, const std::string& message) {}
        virtual bool removePrivateRoomMessage(size_t index) { return false; }
        virtual void addGroupRoomMessage(long long group_id, long long sender_id, MessageType type, const std::string& message) {}
        virtual bool removeGroupRoomMessage(size_t index) { return false; }
    };
}

#endif // !BASE_MAIN_WINDOW_H
