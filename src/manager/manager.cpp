#include "manager.h"

#include <functional>

#include "src/factory/factory.h"
#include "src/mainWindow/baseMainWindow.h"

namespace qls
{
    struct ManagerImpl
    {
        BaseNetwork& network = Factory::getGlobalFactory().getNetwork();
        BaseMainWindow& mainWindow = Factory::getGlobalFactory().getMainWindow();
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

    void Manager::connected_callback()
    {
        m_manager_impl->mainWindow.connected_callback();
    }

    void Manager::disconnected_callback()
    {
        m_manager_impl->mainWindow.disconnected_callback();
    }

    void Manager::received_message(std::string data)
    {
        
    }
}
