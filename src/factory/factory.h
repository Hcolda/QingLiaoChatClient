#ifndef FACTORY_H
#define FACTORY_H

#include <memory>

#include "src/network/network.h"
#include "src/mainWindow/baseMainWindow.h"
#include "src/login/login.h"
#include "src/start/start.h"
#include "src/manager/dataManager.h"

namespace qingliao
{
    struct FactoryImpl;

    class Factory
    {
    protected:
        Factory();

    public:
        static Factory& getGlobalFactory();
        ~Factory();

        // 禁止复制和移动
        Factory(const Factory&) = delete;
        Factory(Factory&&) = delete;

        Factory& operator=(const Factory&) = delete;
        Factory& operator=(Factory&&) = delete;

        BaseNetwork& getNetwork() const;
        BaseMainWindow& getMainWindow() const;
        DataManager& getDataManager() const;

        Login* createNewLoginWidget(QWidget* parent = nullptr);
        Start* createNewStartWidget(QWidget* parent = nullptr);

    private:
        std::shared_ptr<FactoryImpl> m_factory_impl;
    };
}

#endif // !FACTORY_H
