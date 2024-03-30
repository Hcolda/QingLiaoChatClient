#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QPoint>
#include <QMouseEvent>

#include <system_error>

#include "src/mainWindow/baseMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QDialog, public qls::BaseMainWindow
{
    Q_OBJECT

public:
    Login(QWidget *parent = nullptr);
    ~Login();

signals:
    void connected_error_singal(std::error_code);
    void connected_singal();
    void disconnected_singal();

protected slots:
    //界面移动
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void connected_error_slot(std::error_code);
    void connected_slot();
    void disconnected_slot();

protected:
    void connected_callback();
    void disconnected_callback();
    void connected_error_callback(std::error_code);

private:
    Ui::Login *ui;

    bool flag_;
    QPoint _position;
};
#endif // LOGIN_H
