#include "manager.h"

#include <functional>
#include <unordered_map>
#include <shared_mutex>

#include "src/factory/factory.h"
#include "src/mainWindow/baseMainWindow.h"

namespace qingliao
{
    struct ManagerImpl
    {
        BaseNetwork& network = Factory::getGlobalFactory().getNetwork();

        std::unordered_map<std::string,
            qingliao::BaseMainWindow*>   mainWindow_map;
        std::shared_mutex           mainWindow_map_mutex;
    };

    Manager::Manager() :
        m_manager_impl(std::make_shared<ManagerImpl>())
    {
        m_manager_impl->network.add_connected_callback("ManagerConnected",
            std::bind(&Manager::connected_callback, this));
        m_manager_impl->network.add_disconnected_callback("ManagerDisconnected",
            std::bind(&Manager::disconnected_callback, this));
        m_manager_impl->network.add_received_stdstring_callback("ManagerReceivedMessage",
            std::bind(&Manager::received_message, this, std::placeholders::_1));
        m_manager_impl->network.add_connected_error_callback("ManagerConnectedError",
            std::bind(&Manager::connected_error_callback, this, std::placeholders::_1));
    }

    Manager& Manager::getGlobalManager()
    {
        static Manager local_manager;
        return local_manager;
    }

    Manager::~Manager()
    {
        m_manager_impl->network.remove_connected_callback("ManagerConnected");
        m_manager_impl->network.remove_disconnected_callback("ManagerDisconnected");
        m_manager_impl->network.remove_received_stdstring_callback("ManagerReceivedMessage");
    }

    bool Manager::removeMainWindow(const std::string& name)
    {
        std::unique_lock<std::shared_mutex> lock(m_manager_impl->mainWindow_map_mutex);
        auto iter = m_manager_impl->mainWindow_map.find(name);
        if (iter == m_manager_impl->mainWindow_map.cend())
            return false;
        m_manager_impl->mainWindow_map.erase(iter);
        return true;
    }

    bool Manager::addMainWindow(const std::string& name, qingliao::BaseMainWindow* window)
    {
        std::unique_lock<std::shared_mutex> lock(m_manager_impl->mainWindow_map_mutex);
        auto iter = m_manager_impl->mainWindow_map.find(name);
        if (iter != m_manager_impl->mainWindow_map.cend())
            return false;
        m_manager_impl->mainWindow_map[name] = window;
        return true;
    }

    void Manager::connected_callback()
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->mainWindow_map_mutex);
        for (const auto& [_, window] : m_manager_impl->mainWindow_map)
        {
            window->connected_callback();
        }
    }

    void Manager::disconnected_callback()
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->mainWindow_map_mutex);
        for (const auto& [_, window] : m_manager_impl->mainWindow_map)
        {
            window->disconnected_callback();
        }
    }

    void Manager::received_message(std::string data)
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->mainWindow_map_mutex);
        /*for (const auto& [_, window] : m_manager_impl->mainWindow_map)
        {
            window->;
        }*/
    }

    void Manager::connected_error_callback(std::error_code ec)
    {
        std::shared_lock<std::shared_mutex> sl(m_manager_impl->mainWindow_map_mutex);
        for (const auto& [_, window] : m_manager_impl->mainWindow_map)
        {
            window->connected_error_callback(ec);
        }
    }
}
