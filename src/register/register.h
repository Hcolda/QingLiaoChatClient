#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>

#include <QDialog>
#include <QPoint>
#include <QMouseEvent>

#include "src/mainWindow/baseMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Register; }
QT_END_NAMESPACE

class Register : public QDialog, public qingliao::BaseMainWindow
{
    Q_OBJECT

public:
    Register(QWidget* parent = nullptr);
    ~Register();

protected slots:
    //界面移动
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    Ui::Register* ui;

    bool flag_;
    QPoint position_;
};

#endif // !REGISTER_H
