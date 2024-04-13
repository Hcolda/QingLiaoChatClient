#include "factory.h"

#include "src/mainWindow/mainWindow.h"

namespace qingliao
{
    struct FactoryImpl
    {
        Network network;
        MainWindow mainWindow;
        DataManager dataManager;
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

    DataManager& Factory::getDataManager() const
    {
        return m_factory_impl->dataManager;
    }

    Login* Factory::createNewLoginWidget(QWidget* parent)
    {
        return new Login { parent };
    }

    Start* Factory::createNewStartWidget(QWidget* parent)
    {
        return new Start { parent };
    }
}
