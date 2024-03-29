#ifndef STACK_WIDGET_H
#define STACK_WIDGET_H

#include <QStackedWidget>
#include <QMouseEvent>

class StackWidget : public QStackedWidget
{
public:
    StackWidget(QWidget* parent = nullptr);
    ~StackWidget();

    static StackWidget& getGlobalStackWidget();

protected slots:
    //界面移动
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    
private:
    bool flag_;
    QPoint _position;
};

#endif // !STACK_WIDGET_H
