#include "startlabel.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyle>

StartLabel::StartLabel(QWidget* parent) :
    QLabel(parent)
{
}

void StartLabel::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QBrush("#F7F7F7"));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 5, 5);
    QLabel::paintEvent(event);
}