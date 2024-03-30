#include "mainWindow.h"
#include "./ui_MainWindow.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <regex>

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    flag_(false)
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
        //ui->widget->setGraphicsEffect(shadow);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    //界面移动
    int num = (event->globalPos() - this->pos()).y();
    if (event->button() == Qt::LeftButton)
    {
        flag_ = true;
        _position = std::move(event->globalPos() - this->pos());
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (flag_)
    {
        this->move(event->globalPos() - _position);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    flag_ = false;
}