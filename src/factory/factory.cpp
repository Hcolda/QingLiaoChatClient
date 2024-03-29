#include "factory.h"

namespace qls
{
    struct FactoryImpl
    {
        Network network;
        BaseMainWindow mainWindow;
    };

    Factory::Factory() :
        m_factory_impl(std::make_shared<FactoryImpl>())
    {
    }

    Factory::~Factory()
    {
    }

    Factory& Factory::getGlobalFactory()
    {
        static Factory local_factory;
        return local_factory;
    }

    BaseNetwork& Factory::getNetwork() const
    {
        return m_factory_impl->network;
    }

    BaseMainWindow& Factory::getMainWindow() const
    {
        return m_factory_impl->mainWindow;
    }

    std::shared_ptr<Login> Factory::createNewLoginWidget()
    {
        return std::make_shared<Login>();
    }

    std::shared_ptr<Start> Factory::createNewStartWidget()
    {
        return std::make_shared<Start>();
    }
}
