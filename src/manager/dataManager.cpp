#include "dataManager.h"

#include <atomic>
#include <unordered_map>
#include <shared_mutex>

#include "src/network/dataPackage.h"
#include "src/factory/factory.h"
#include "src/parser/Json.h"

namespace qingliao
{
    struct DataManagerImpl
    {
        std::atomic<bool> is_login = false;


    };


    DataManager::DataManager() :
        m_impl(std::make_shared<DataManagerImpl>()) {}

    DataManager::~DataManager() {}

    bool DataManager::signUp(const std::string& email, const std::string& password)
    {
        qjson::JObject json(qjson::JValueType::JDict);
        json["function"] = "register";
        json["parameters"]["email"] = email;
        json["parameters"]["password"] = password;

        auto& network = Factory::getGlobalFactory().getNetwork();
        auto pack = network.send_data_with_result_n_option(qjson::JWriter::fastWrite(json), [](std::shared_ptr<DataPackage>& pack) { pack->type = 1; });
        try
        {
            qjson::JObject rejson = qjson::JParser::fastParse(pack->getData());
            if (rejson["status"].getString() != "success") return false;
        }
        catch (...)
        {
            return false;
        }
        return true;
    }

    bool DataManager::signIn(long long user_id, const std::string& password)
    {
        qjson::JObject json(qjson::JValueType::JDict);
        json["function"] = "login";
        json["parameters"]["user_id"] = user_id;
        json["parameters"]["password"] = password;

        auto& network = Factory::getGlobalFactory().getNetwork();
        auto pack = network.send_data_with_result_n_option(qjson::JWriter::fastWrite(json), [](std::shared_ptr<DataPackage>& pack) { pack->type = 1; });
        try
        {
            qjson::JObject rejson = qjson::JParser::fastParse(pack->getData());
            if (rejson["state"].getString() != "success") return false;
        }
        catch (...)
        {
            return false;
        }

        m_impl->is_login = true;
        return true;
    }

    bool DataManager::addPrivateRoom(long long user_id)
    {
        return false;
    }

    bool DataManager::romovePrivateRoom(long long user_id)
    {
        return false;
    }

    bool DataManager::addGroupRoom(long long room_id)
    {
        return false;
    }

    bool DataManager::removeGroupRoom(long long room_id)
    {
        return false;
    }

    void DataManager::addPrivateRoomMessage(long long user_id, MessageType type, const std::string& message)
    {
    }

    bool DataManager::removePrivateRoomMessage(size_t index)
    {
        return false;
    }

    void DataManager::addGroupRoomMessage(long long group_id, long long sender_id, MessageType type, const std::string& message)
    {
    }

    bool DataManager::removeGroupRoomMessage(size_t index)
    {
        return false;
    }

}