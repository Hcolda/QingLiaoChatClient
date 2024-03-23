#include "manager.h"

#include "src/factory/factory.h"

namespace qls
{
    struct ManagerImpl
    {
        BaseNetwork& network = Factory::getGlobalFactory().getNetwork();
    };

    Manager::Manager()
    {
    }

    Manager::~Manager()
    {
    }
}
