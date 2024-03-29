#ifndef FACTORY_H
#define FACTORY_H

#include <memory>

#include "src/network/network.h"
#include "src/mainWindow/baseMainWindow.h"
#include "src/login/login.h"
#include "src/start/start.h"

namespace qls
{
    struct FactoryImpl;

    class Factory
    {
    protected:
        Factory();

    public:
        static Factory& getGlobalFactory();
        ~Factory();

        BaseNetwork& getNetwork() const;
        BaseMainWindow& getMainWindow() const;

        std::shared_ptr<Login> createNewLoginWidget();
        std::shared_ptr<Start> createNewStartWidget();

    private:
        std::shared_ptr<FactoryImpl> m_factory_impl;
    };
}

#endif // !FACTORY_H
