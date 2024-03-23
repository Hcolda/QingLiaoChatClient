#ifndef MANAGER_H
#define MANAGER_H

#include <memory>

#include "manager.h"

namespace qls
{
    struct ManagerImpl;
    
    class Manager
    {
    public:
        Manager();
        ~Manager();

    private:
        std::shared_ptr<ManagerImpl> m_manager_impl;
    };
}

#endif // !MANAGER_H
