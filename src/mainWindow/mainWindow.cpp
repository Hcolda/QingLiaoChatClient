#include "mainWindow.h"
#include "./ui_MainWindow.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

#include <cstdlib>
#include <regex>

#include "src/factory/factory.h"
#include "src/manager/manager.h"
#include "src/start/start.h"
#include "src/login/login.h"

struct MainWindowImpl
{
    qingliao::SendPrivateRoomMessageFunc sendPrivateRoomMessageFunction;
    qingliao::SendGroupRoomMessageFunc   sendGroupRoomMessageFunction;
    qingliao::SendCommonMessageFunc      sendCommonMessageFunction;
};

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    flag_(false),
    mainwindow_impl_(std::make_shared<MainWindowImpl>())
{
    ui->setupUi(this);

    //界面设置
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    //关闭按钮
    QObject::connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::close);
    //最小化按钮
    QObject::connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::showMinimized);

    // 设置阴影
    {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setOffset(0, 0);
        //设置阴影颜色
        shadow->setColor(QColor("#004D40"));
        //设置阴影圆角
        shadow->setBlurRadius(6);
        //给嵌套QWidget设置阴影
        ui->widget->setGraphicsEffect(shadow);
    }
}

MainWindow::~MainWindow()
{
    /*auto& manager = qingliao::Manager::getGlobalManager();
    auto& network = qingliao::Factory::getGlobalFactory().getNetwork();
    manager.removeMainWindow("MainWindow");
    network.stop();*/
    delete ui;
}

void MainWindow::run()
{
    Start start;
    if (start.exec() != QDialog::Accepted)
    {
        qingliao::BaseNetwork& network = qingliao::Factory::getGlobalFactory().getNetwork();
        network.stop();
        return;
    }

    Login login;
    if (login.exec() != QDialog::Accepted)
    {
        qingliao::BaseNetwork& network = qingliao::Factory::getGlobalFactory().getNetwork();
        network.stop();
        return;
    }

    qingliao::Manager& manager = qingliao::Manager::getGlobalManager();
    manager.addMainWindow("MainWindow", this);
    show();
}

bool MainWindow::addPrivateRoom(long long user_id)
{
    return false;
}

bool MainWindow::romovePrivateRoom(long long user_id)
{
    return false;
}

bool MainWindow::addGroupRoom(long long room_id)
{
    return false;
}

bool MainWindow::removeGroupRoom(long long room_id)
{
    return false;
}

void MainWindow::addPrivateRoomMessage(long long user_id, qingliao::MessageType type, const std::string& message)
{
}

bool MainWindow::removePrivateRoomMessage(size_t index)
{
    return false;
}

void MainWindow::addGroupRoomMessage(long long group_id, long long sender_id, qingliao::MessageType type, const std::string& message)
{
}

bool MainWindow::removeGroupRoomMessage(size_t index)
{
    return false;
}

void MainWindow::setPrivateRoomMessageCallback(qingliao::SendPrivateRoomMessageFunc func)
{
    mainwindow_impl_->sendPrivateRoomMessageFunction = func;
}

void MainWindow::setGroupRoomMessageCallback(qingliao::SendGroupRoomMessageFunc func)
{
    mainwindow_impl_->sendGroupRoomMessageFunction = func;
}

void MainWindow::setCommonMessageCallback(qingliao::SendCommonMessageFunc func)
{
    mainwindow_impl_->sendCommonMessageFunction = func;
}

void MainWindow::connected_callback()
{
}

void MainWindow::disconnected_callback()
{
}

void MainWindow::connected_error_callback(std::error_code)
{
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    //界面移动
    int num = (event->globalPos() - this->pos()).y();
    if (event->button() == Qt::LeftButton)
    {
        flag_ = true;
        position_ = std::move(event->globalPos() - this->pos());
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (flag_)
    {
        this->move(event->globalPos() - position_);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    flag_ = false;
}