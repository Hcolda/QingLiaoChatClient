#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <memory>

#include <QMainWindow>
#include <QtWidgets/QWidget>
#include <QPoint>
#include <QMouseEvent>

#include "baseMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct MainWindowImpl;

class MainWindow : public QMainWindow, public qls::BaseMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool addPrivateRoom(long long user_id);
    bool romovePrivateRoom(long long user_id);
    bool addGroupRoom(long long roon_id);
    bool removeGroupRoom(long long roon_id);

    void addPrivateRoomMessage(long long user_id, qls::MessageType type, const std::string& message);
    bool removePrivateRoomMessage(size_t index);
    void addGroupRoomMessage(long long group_id, long long sender_id, qls::MessageType type, const std::string& message);
    bool removeGroupRoomMessage(size_t index);

    void setPrivateRoomMessageCallback(qls::SendPrivateRoomMessageFunc func);
    void setGroupRoomMessageCallback(qls::SendGroupRoomMessageFunc func);
    void setCommonMessageCallback(qls::SendCommonMessageFunc func);

    void connected_callback();
    void disconnected_callback();
    void connected_error_callback(std::error_code);

protected slots:
    //界面移动
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

private:
    Ui::MainWindow* ui;

    std::shared_ptr<MainWindowImpl> mainwindow_impl_;
    bool flag_;
    QPoint position_;
};

#endif // !MAIN_WINDOW_H
