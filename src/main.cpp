#include "src/factory/factory.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSharedMemory>

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

    auto& main_window = qingliao::Factory::getGlobalFactory().getMainWindow();
    main_window.run();
    return a.exec();
}
