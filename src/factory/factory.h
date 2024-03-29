#ifndef FACTORY_H
#define FACTORY_H

#include <memory>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <queue>
#include <thread>
#include <utility>
#include <atomic>

#include "src/network/network.h"
#include "src/mainWindow/baseMainWindow.h"
#include "src/login/login.h"
#include "src/start/start.h"

namespace qls
{
    class Runner;
    struct FactoryImpl;

    class Factory
    {
    protected:
        Factory();

    public:
        static Factory& getGlobalFactory();
        ~Factory();

        BaseNetwork&    getNetwork() const;
        BaseMainWindow& getMainWindow() const;
        Runner&         getRunner() const;

        Login* createNewLoginWidget(QWidget* parent = nullptr);
        Start* createNewStartWidget(QWidget* parent = nullptr);

    private:
        std::shared_ptr<FactoryImpl> m_factory_impl;
    };

    class Runner
    {
    public:
        Runner() = default;

        ~Runner()
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (!m_queue.empty())
                {
                    m_queue.pop();
                }
            }

            m_cv.notify_all();
        }

        template<typename Func, typename... Args>
        void add_task(Func&& func, Args&&... args)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_queue.push(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
            }

            m_cv.notify_all();
        }

        void run_loop()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [&]() { return !m_queue.empty(); });

                if (m_queue.empty())
                {
                    return;
                }

                auto func = std::move(m_queue.front());
                lock.unlock();
                func();
            }
        }

    private:
        std::queue<std::function<void()>>   m_queue;
        std::mutex                          m_mutex;
        std::condition_variable             m_cv;
    };
}

#endif // !FACTORY_H
