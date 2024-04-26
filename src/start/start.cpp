#include "start.h"
#include "./ui_start.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <regex>

#include "src/manager/manager.h"
#include "src/factory/factory.h"
#include "src/network/network.h"

extern qingliao::Factory clientFactory;

Start::Start(QWidget* parent):
    QDialog(parent),
    ui(new Ui::Start),
    flag_(false)
{
    ui->setupUi(this);

    //界面设置
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    QObject::connect(this, &Start::accepted_signal, this, &Start::accepted_slot);
    QObject::connect(this, &Start::rejected_signal, this, &Start::rejected_slot);

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

    auto manager = clientFactory.getManager();
    auto network = clientFactory.getNetwork();
    manager->addMainWindow("Start", this);
    network->connect();
}

Start::~Start()
{
    auto manager = clientFactory.getManager();
    manager->removeMainWindow("Start");
    delete ui;
}

void Start::connected_callback()
{
    emit accepted_signal();
}

void Start::disconnected_callback()
{
    emit rejected_signal();
}

void Start::connected_error_callback(std::error_code)
{
    emit rejected_signal();
}

void Start::mousePressEvent(QMouseEvent* event)
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

void Start::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (flag_)
    {
        this->move(event->globalPos() - position_);
        event->accept();
    }
}

void Start::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    flag_ = false;
}

void Start::accepted_slot()
{
    accept();
}

void Start::rejected_slot()
{
    ui->status->setText("Could not connect to server");
    ui->status->setStyleSheet("color: red;");
}
