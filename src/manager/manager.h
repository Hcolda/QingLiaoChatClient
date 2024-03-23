#ifndef MANAGER_H
#define MANAGER_H

#include <memory>
#include <string>

#include "manager.h"

namespace qls
{
    struct ManagerImpl;
    
    class Manager
    {
    protected:
        Manager();

    public:
        static Manager& getGlobalManager();
        ~Manager();

    protected:
        void connected_callback();
        void disconnected_callback();
        void received_message(std::string);

    private:
        std::shared_ptr<ManagerImpl> m_manager_impl;
    };
}

#endif // !MANAGER_H
