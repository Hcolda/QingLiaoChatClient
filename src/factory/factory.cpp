#include "factory.h"

namespace qls
{
    struct FactoryImpl
    {
        Network network;
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
}
