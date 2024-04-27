#include "register.h"
#include "./ui_register.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

#include <format>

#include "regexMatch.hpp"
#include "src/messageBox/messageBox.hpp"
#include "src/manager/manager.h"
#include "src/factory/factory.h"
#include "src/manager/dataManager.h"

extern qingliao::Factory clientFactory;

static void addSetVisableButton(QLineEdit* parent)
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

    QWidgetAction* widgetAction = new QWidgetAction(parent);
    widgetAction->setDefaultWidget(btn);

    parent->addAction(widgetAction, QLineEdit::TrailingPosition);
    btn->setVisible(false);

    QObject::connect(parent, &QLineEdit::textChanged, [=]() {
        if (parent->text().size())
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
    QObject::connect(btn, &QPushButton::clicked, [=]() {
        static bool openShowPassword = false;

        if (!openShowPassword)
        {
            openShowPassword = true;
            parent->setEchoMode(QLineEdit::Normal);
            btn->setIcon(QIcon(":Login/img/eye1.png"));
        }
        else
        {
            openShowPassword = false;
            parent->setEchoMode(QLineEdit::Password);
            btn->setIcon(QIcon(":Login/img/eye2.png"));
        }

        });
}

Register::Register(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Register),
    flag_(false)
{
    ui->setupUi(this);

    //界面设置
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    // 显示密码按钮
    // addSetVisableButton(ui->password_lineEdit);
    // addSetVisableButton(ui->password_lineEdit_2);

    ui->password_prograssBar->setVisible(false);
    ui->email_match_label->setVisible(false);
    ui->password_match_label->setVisible(false);

    // 检测邮箱格式
    QObject::connect(ui->email_lineEdit, &QLineEdit::textChanged, [=]() {
        auto text = ui->email_lineEdit->text().toStdString();
        if (text.empty() || qingliao::RegexMatch::emailMatch(text))
        {
            ui->email_match_label->setVisible(false);
            return;
        }
        ui->email_match_label->setVisible(true);
        });

    // 检测密码强弱
    QObject::connect(ui->password_lineEdit, &QLineEdit::textChanged, [=]() {
        static std::regex
            medium_password_regex("(?!^\\d+$)(?!^[a-zA-Z]+$).*$"),
            strong_password_regex("(?=^.{10,}$)((?=.*\\d)|(?=.*\\W+))(?![.\\n])(?=.*[A-Z])(?=.*[a-z]).*$");
        auto text = ui->password_lineEdit->text().toStdString();

        if (text.empty())
        {
            ui->password_prograssBar->setVisible(false);
            return;
        }

        // 检测强弱
        if (text.size() <= 6)
        {
            ui->password_prograssBar->setStyleSheet(R"(
QProgressBar{
	color: #FF1744;
} 

QProgressBar::chunk {
	background: #FF1744;
	margin-top: 8%;
	margin-bottom: 8%;
	margin-right: 10%;
})");
            ui->password_prograssBar->setFormat("弱");
            ui->password_prograssBar->setValue(33);
            ui->password_prograssBar->setVisible(true);
            return;
        }

        if (std::regex_match(text, strong_password_regex))
        {
            ui->password_prograssBar->setStyleSheet(R"(
QProgressBar{
	color: #00BFA5;
} 

QProgressBar::chunk {
	background: #00BFA5;
	margin-top: 8%;
	margin-bottom: 8%;
	margin-right: 50%;
})");
            ui->password_prograssBar->setFormat("强");
            ui->password_prograssBar->setValue(100);
        }
        else if (std::regex_match(text, medium_password_regex))
        {
            ui->password_prograssBar->setStyleSheet(R"(
QProgressBar{
	color: #FFB300;
} 

QProgressBar::chunk {
	background: #FFB300;
	margin-top: 8%;
	margin-bottom: 8%;
	margin-right: 30%;
})");
            ui->password_prograssBar->setFormat("中");
            ui->password_prograssBar->setValue(66);
        }
        else
        {
            ui->password_prograssBar->setStyleSheet(R"(
QProgressBar{
	color: #FF1744;
} 

QProgressBar::chunk {
	background: #FF1744;
	margin-top: 8%;
	margin-bottom: 8%;
	margin-right: 10%;
})");
            ui->password_prograssBar->setFormat("弱");
            ui->password_prograssBar->setValue(33);
        }

        ui->password_prograssBar->setVisible(true);
        });

    // 检测密码是否一致
    QObject::connect(ui->password_lineEdit_2, &QLineEdit::textChanged, [=]() {
        auto text = ui->password_lineEdit->text().toStdString();
        auto test_text = ui->password_lineEdit_2->text().toStdString();
        if (text.empty() || text == test_text)
        {
            ui->password_match_label->setVisible(false);
        }
        else
        {
            ui->password_match_label->setVisible(true);
        }
        });

    QObject::connect(ui->close_button, &QPushButton::clicked, this, &Register::close);
    QObject::connect(ui->register_button, &QPushButton::clicked, this, [=]() {
        if (!ui->email_lineEdit->text().size())
        {
            WarningBox box("警告", "邮箱为空", QMessageBox::StandardButton::Ok, this);
            box.exec();
            return;
        }
        if (!ui->password_lineEdit->text().size())
        {
            WarningBox box("警告", "密码为空", QMessageBox::StandardButton::Ok, this);
            box.exec();
            return;
        }
        if (!ui->password_lineEdit_2->text().size())
        {
            WarningBox box("警告", "确认密码为空", QMessageBox::StandardButton::Ok, this);
            box.exec();
            return;
        }
        if (!qingliao::RegexMatch::emailMatch(ui->email_lineEdit->text().toStdString()))
        {
            WarningBox box("警告", "邮箱格式错误", QMessageBox::StandardButton::Ok, this);
            box.exec();
            return;
        }
        if (ui->password_lineEdit->text() != ui->password_lineEdit_2->text())
        {
            WarningBox box("警告", "密码不一致", QMessageBox::StandardButton::Ok, this);
            box.exec();
            return;
        }

        {
            long long user_id = 0;
            auto dataManager = clientFactory.getDataManager();
            if (dataManager->signUp(
                ui->email_lineEdit->text().toStdString(),
                ui->password_lineEdit->text().toStdString(),
                user_id))
            {
                BaseMessageBox box(QMessageBox::Icon::Information,
                    "信息", QString::fromStdString(std::format("注册成功！\n请记住你的唯一UID：{}", user_id)),
                    QMessageBox::Ok, this);
                box.exec();
                accept();
                return;
            }
        }
        });

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

    auto manager = clientFactory.getManager();
    manager->addMainWindow("Register", this);
}

Register::~Register()
{
    auto manager = clientFactory.getManager();
    manager->removeMainWindow("Register");
    delete ui;
}

void Register::mousePressEvent(QMouseEvent* event)
{
    //界面移动
    int num = (event->globalPos() - this->pos()).y();
    if (event->button() == Qt::LeftButton && num < 50 && num > 10)
    {
        flag_ = true;
        position_ = std::move(event->globalPos() - this->pos());
        event->accept();
    }
}

void Register::mouseMoveEvent(QMouseEvent* event)
{
    //界面移动
    if (flag_)
    {
        this->move(event->globalPos() - position_);
        event->accept();
    }
}

void Register::mouseReleaseEvent(QMouseEvent* event)
{
    //界面移动
    flag_ = false;
}
