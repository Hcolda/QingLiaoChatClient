﻿#include "login.h"
#include "./ui_login.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <regex>

#include "src/messageBox/messageBox.hpp"
#include "src/manager/manager.h"
#include "src/factory/factory.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
    , _flag(false)
{
    ui->setupUi(this);

    //界面设置
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    //关闭按钮
    connect(ui->pushButton, &QPushButton::clicked, this, &Login::close);
    //最小化按钮
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Login::showMinimized);

    connect(this, &Login::connected_error_singal, this, &Login::connected_error_slot);
    connect(this, &Login::connected_singal, this, &Login::connected_slot);
    connect(this, &Login::disconnected_singal, this, &Login::disconnected_slot);

    //显示密码
    {
        QPushButton* btn = new QPushButton;
        btn->setIcon(QIcon(":Login/img/eye2.png"));
        btn->setStyleSheet(R"+*(
QPushButton{
    background:rgb(247,247,247);
    border-radius:5px;
    border:0px;
}

QPushButton:hover{
    background:rgb(200,200,200);
}

QPushButton:pressed{
    background:rgb(200,200,200);
}
    )+*");
        btn->setCursor(Qt::PointingHandCursor);

        QWidgetAction* widgetAction = new QWidgetAction(ui->lineEdit_2);
        widgetAction->setDefaultWidget(btn);

        ui->lineEdit_2->addAction(widgetAction, QLineEdit::TrailingPosition);
        btn->setVisible(false);

        connect(ui->lineEdit_2, &QLineEdit::textChanged, [=]() {
            if (ui->lineEdit_2->text().size())
            {
                btn->setVisible(true);
                btn->setCursor(Qt::PointingHandCursor);
                btn->setEnabled(true);
            }
            else
            {
                btn->setVisible(false);
            }
            });
        connect(btn, &QPushButton::clicked, [=]() {
            static bool openShowPassword = false;

            if (!openShowPassword)
            {
                openShowPassword = true;
                ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
                btn->setIcon(QIcon(":Login/img/eye1.png"));
            }
            else
            {
                openShowPassword = false;
                ui->lineEdit_2->setEchoMode(QLineEdit::Password);
                btn->setIcon(QIcon(":Login/img/eye2.png"));
            }

            });

        // 登录
        connect(ui->pushButton_3, &QPushButton::clicked, this, [=]() -> void {
            // 判断
            if (!ui->lineEdit->text().size())
            {
                WarningBox box("警告", "账户名为空！", QMessageBox::StandardButton::Ok, this);
                box.exec();
            }
            else if (!ui->lineEdit_2->text().size())
            {
                WarningBox box("警告", "密码为空！", QMessageBox::StandardButton::Ok, this);
                box.exec();
            }

            return;
            });

        // 注册
        connect(ui->pushButton_4, &QPushButton::clicked, this, [=]() -> void {
            return;
            });
    }

    // 设置阴影
    {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setOffset(0, 0);
        //设置阴影颜色
        shadow->setColor(QColor("#004D40"));
        //设置阴影圆角
        shadow->setBlurRadius(6);
        //给嵌套QWidget设置阴影
        ui->frame->setGraphicsEffect(shadow);
    }

    qls::Manager& manager = qls::Manager::getGlobalManager();
    qls::BaseNetwork& network = qls::Factory::getGlobalFactory().getNetwork();
    manager.addMainWindow("Login", this);
    network.connect();
    
    // 毛玻璃效果
    /*QGraphicsBlurEffect* effect = new QGraphicsBlurEffect(this);
    effect->setBlurRadius(5);
    ui->frame->setGraphicsEffect(effect);*/
}

Login::~Login()
{
    qls::Manager& manager = qls::Manager::getGlobalManager();
    manager.removeMainWindow("Login");
    delete ui;
}

void Login::mousePressEvent(QMouseEvent* event)
{
    //界面移动
    int num = (event->globalPos() - this->pos()).y();
    if (event->button() == Qt::LeftButton && num < 50 && num > 10)
    {
        _flag = true;
        _position = std::move(event->globalPos() - this->pos());
        event->accept();
    }
}

void Login::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (_flag)
    {
        this->move(event->globalPos() - _position);
        event->accept();
    }
}

void Login::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    _flag = false;
}

void Login::connected_error_slot(std::error_code)
{
    WarningBox box("警告", "无法连接服务器！", QMessageBox::StandardButton::Ok, this);
    box.exec();
}

void Login::connected_slot()
{
    
}

void Login::disconnected_slot()
{

}

void Login::connected_callback()
{
    emit connected_singal();
}

void Login::disconnected_callback()
{
    emit disconnected_singal();
}

void Login::connected_error_callback(std::error_code ec)
{
    emit connected_error_singal(ec);
}
