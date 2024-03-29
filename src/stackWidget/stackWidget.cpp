#include "stackWidget.h"

#include "src/factory/factory.h"

#include <QScreen>
#include <QRect>
#include <QGuiApplication>

StackWidget::StackWidget(QWidget* parent) :
    QStackedWidget(parent),
    flag_(false)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect rect = screen->geometry();

    this->resize(rect.width(), rect.height());

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    auto& factory = qls::Factory::getGlobalFactory();
    addWidget(factory.createNewStartWidget());
    addWidget(factory.createNewStartWidget());
}

StackWidget::~StackWidget()
{
}

StackWidget& StackWidget::getGlobalStackWidget()
{
    static StackWidget local_stack_widget;
    return local_stack_widget;
}

void StackWidget::mousePressEvent(QMouseEvent* event)
{
    //界面移动
    if (event->button() == Qt::LeftButton)
    {
        flag_ = true;
        _position = std::move(event->globalPos() - this->pos());
        event->accept();
    }
}

void StackWidget::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (flag_)
    {
        this->move(event->globalPos() - _position);
        event->accept();
    }
}

void StackWidget::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    flag_ = false;
    event->accept();
}

