#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QList>
#include <QAbstractButton>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

#include <format>

class MessageBox : public QMessageBox
{
    Q_OBJECT

public:
    MessageBox( QMessageBox::Icon icon,
                const QString& title,
                const QString& text,
                QMessageBox::StandardButtons buttons = NoButton,
                QWidget* parent = nullptr,
                QString backgroundColor = "#F7F7F7",
                QString shadowColor = "#004D40") :
        QMessageBox(icon, title, text, buttons, parent, Qt::FramelessWindowHint),
        flag_(false),
        _backgroundColor(backgroundColor)
    {
        //界面设置
        this->setAttribute(Qt::WA_TranslucentBackground);

        auto styleString = std::format(R"(
background: {};
border-radius: 15px;
color: #212121;
border: {};)", backgroundColor.toStdString(), backgroundColor.toStdString());

        this->setStyleSheet(QString::fromStdString(styleString));
        this->setMinimumSize(300, 100);

        auto list = this->buttons();
        for (auto& bi : list)
        {
            bi->setStyleSheet(R"(
QPushButton{
    background:#00BFA5;
    border-radius:10px;
    color:rgb(247,247,247);
}

QPushButton:hover{
    background:#00796B;
}

QPushButton:pressed{
    background:#00796B;
})");
            bi->setMinimumSize(50, 20);
        }

        {
            QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
            shadow->setOffset(2, 2);
            //设置阴影颜色
            shadow->setColor(QColor(shadowColor));
            //设置阴影圆角
            shadow->setBlurRadius(5);
            //给嵌套QWidget设置阴影
            this->setGraphicsEffect(shadow);
        }
    }

    ~MessageBox() = default;

protected slots:

    //界面移动
    void mousePressEvent(QMouseEvent* event)
    {
        //界面移动
        if (event->button() == Qt::LeftButton)
        {
            flag_ = true;
            position_ = std::move(event->globalPos() - this->pos());
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent* event)
    {
        //界面移动
        if (flag_)
        {
            this->move(event->globalPos() - position_);
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event)
    {
        //界面移动
        flag_ = false;
    }

    void paintEvent(QPaintEvent* event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter.setBrush(QBrush(_backgroundColor));
        painter.setPen(Qt::transparent);
        QRect rect = this->rect();
        rect.setWidth(rect.width() - 5);
        rect.setHeight(rect.height() - 5);
        painter.drawRoundedRect(rect, 10, 10);
        QWidget::paintEvent(event);
    }
    
private:
    bool flag_;
    QPoint position_;
    QColor _backgroundColor;
};

class WarningBox : public MessageBox
{
    Q_OBJECT

public:
    WarningBox(const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons = NoButton,
        QWidget* parent = nullptr) :
        MessageBox(QMessageBox::Icon::Warning,
            title, text, buttons, parent, "#FFEBEE", "#D50000")
    {
        auto list = this->buttons();
        for (auto& bi : list)
        {
            bi->setStyleSheet(R"(
QPushButton{
    background:#F44336;
    border-radius:10px;
    color:rgb(247,247,247);
}

QPushButton:hover{
    background:#C62828;
}

QPushButton:pressed{
    background:#C62828;
})");
        }
    }
    ~WarningBox() = default;
};

#endif // !MESSAGE_BOX_HPP
