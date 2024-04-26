#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <string>

#include "src/network/network.h"
#include "src/mainWindow/baseMainWindow.h"

namespace qingliao
{
    struct ManagerImpl;
    
    class Manager
    {
    public:
        Manager(std::weak_ptr<qingliao::BaseNetwork>);
        ~Manager();

        bool addMainWindow(const std::string&, qingliao::BaseMainWindow*);
        bool removeMainWindow(const std::string&);

    protected:
        void connected_callback();
        void disconnected_callback();
        void connected_error_callback(std::error_code);
        void received_message(std::string);

    private:
        std::shared_ptr<ManagerImpl> m_manager_impl;
    };
}

#endif // !MANAGER_H
