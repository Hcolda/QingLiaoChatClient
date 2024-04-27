#include "src/factory/factory.h"
#include "src/mainWindow/mainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSharedMemory>

#include <memory>

// 全局变量
qingliao::Factory clientFactory;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QingLiaoClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    /*QSharedMemory shared("QingLiaoClient");
    if (!shared.create(1))
    {
        return 0;
    }*/

    std::shared_ptr<qingliao::BaseMainWindow>
        mainWindow(clientFactory.createNewMainWindow());
    try
    {
        mainWindow->run();
    }
    catch (...)
    {
        return 0;
    }

    return a.exec();
}
