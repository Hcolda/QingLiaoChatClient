#ifndef FACTORY_H
#define FACTORY_H

#include <memory>

#include "src/network/network.h"

namespace qls
{
    struct FactoryImpl;

    class Factory
    {
    protected:
        Factory();

    public:
        ~Factory();

        static Factory& getGlobalFactory();

        BaseNetwork& getNetwork() const;

    private:
        std::shared_ptr<FactoryImpl> m_factory_impl;
    };
}

#endif // !FACTORY_H
