#ifndef FACTORY_H
#define FACTORY_H

#include <memory>

#include "src/network/network.h"
#include "src/mainWindow/baseMainWindow.h"
#include "src/login/login.h"
#include "src/start/start.h"
#include "src/manager/dataManager.h"
#include "src/manager/manager.h"

namespace qingliao
{
    struct FactoryImpl;

    class Factory
    {
    public:
        Factory();
        ~Factory();

        // 禁止复制和移动
        Factory(const Factory&) = delete;
        Factory(Factory&&) = delete;

        Factory& operator=(const Factory&) = delete;
        Factory& operator=(Factory&&) = delete;

        std::shared_ptr<BaseNetwork> getNetwork() const;
        std::shared_ptr<DataManager> getDataManager() const;
        std::shared_ptr<Manager> getManager() const;

        BaseMainWindow* createNewMainWindow(QWidget* parent = nullptr);
        Login* createNewLoginWidget(QWidget* parent = nullptr);
        Start* createNewStartWidget(QWidget* parent = nullptr);

    private:
        std::shared_ptr<FactoryImpl> m_factory_impl;
    };
}

#endif // !FACTORY_H
