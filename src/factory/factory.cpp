#include "factory.h"

#include "src/mainWindow/mainWindow.h"

namespace qingliao
{
    struct FactoryImpl
    {
        std::shared_ptr<Network>     network;
        std::shared_ptr<DataManager> dataManager;
        std::shared_ptr<Manager>     manager;

        FactoryImpl() :
            network(std::make_shared<Network>()),
            dataManager(std::make_shared<DataManager>())
        {
            manager = std::make_shared<Manager>(network);
        }
    };

    Factory::Factory() :
        m_factory_impl(std::make_shared<FactoryImpl>())
    {
    }

    Factory::~Factory() {}

    std::shared_ptr<BaseNetwork> Factory::getNetwork() const
    {
        return m_factory_impl->network;
    }

    std::shared_ptr<DataManager> Factory::getDataManager() const
    {
        return m_factory_impl->dataManager;
    }

    std::shared_ptr<Manager> Factory::getManager() const
    {
        return m_factory_impl->manager;
    }

    BaseMainWindow* Factory::createNewMainWindow(QWidget* parent)
    {
        return new MainWindow { parent };
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
