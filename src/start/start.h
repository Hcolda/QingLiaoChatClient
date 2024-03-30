#ifndef START_H
#define START_H

#include <QDialog>
#include <QPoint>
#include <QMouseEvent>

#include "src/mainWindow/baseMainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Start; }
QT_END_NAMESPACE

class Start: public QDialog, public qingliao::BaseMainWindow
{
    Q_OBJECT

public:
    Start(QWidget* parent = nullptr);
    ~Start();

    void connected_callback();
    void disconnected_callback();
    void connected_error_callback(std::error_code);

signals:
    void accepted_signal();
    void rejected_signal();

protected slots:
    //界面移动
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void accepted_slot();
    void rejected_slot();

private:
    Ui::Start* ui;

    bool flag_;
    QPoint position_;
};


#endif // !START_H
