#ifndef START_H
#define START_H

#include <QDialog>
#include <QPoint>
#include <QMouseEvent>

#include "src/mainWindow/baseMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Start; }
QT_END_NAMESPACE

class Start: public QDialog, public qingliao::BaseMainWindow
{
    Q_OBJECT

public:
    Start(QWidget* parent = nullptr);
    ~Start();

protected slots:
    //界面移动
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    Ui::Start* ui;

    bool flag_;
    QPoint position_;
};


#endif // !START_H
