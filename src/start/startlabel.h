#ifndef START_LABEL_H
#define START_LABEL_H

#include <QWidget>
#include <QLabel>
#include <QPaintEvent>

class StartLabel : public QLabel
{
    Q_OBJECT

public:
    StartLabel(QWidget* parent = nullptr);
    ~StartLabel() = default;

protected slots:
    void paintEvent(QPaintEvent* event);
};

#endif // !START_LABEL_H
