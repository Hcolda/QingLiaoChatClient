#include "start.h"
#include "./ui_start.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <regex>

Start::Start(QWidget* parent):
    QWidget(parent),
    ui(new Ui::Start),
    flag_(false)
{
    ui->setupUi(this);

    //界面设置
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

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

Start::~Start()
{
    delete ui;
}

void Start::mousePressEvent(QMouseEvent* event)
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

void Start::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (flag_)
    {
        this->move(event->globalPos() - _position);
        event->accept();
    }
}

void Start::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    flag_ = false;
}
