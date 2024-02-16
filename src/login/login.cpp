#include "login.h"
#include "./ui_login.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QWidgetAction>
#include <QPushButton>
#include <QLineEdit>
#include <regex>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
	, _flag(false)
{
    ui->setupUi(this);

	//界面设置
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_TranslucentBackground);

	//关闭按钮
	connect(ui->pushButton, &QPushButton::clicked, this, &Login::close);
	//最小化按钮
	connect(ui->pushButton_2, &QPushButton::clicked, this, &Login::showMinimized);

	//显示密码
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

		QWidgetAction* widgetAction = new QWidgetAction(ui->lineEdit_2);
		widgetAction->setDefaultWidget(btn);

		ui->lineEdit_2->addAction(widgetAction, QLineEdit::TrailingPosition);
		btn->setVisible(false);

		connect(ui->lineEdit_2, &QLineEdit::textChanged, [=]() {
			if (ui->lineEdit_2->text().size())
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
		connect(btn, &QPushButton::clicked, [=]() {
			static bool openShowPassword = false;

			if (!openShowPassword)
			{
				openShowPassword = true;
				ui->lineEdit_2->setEchoMode(QLineEdit::Normal);
				btn->setIcon(QIcon(":Login/img/eye1.png"));
			}
			else
			{
				openShowPassword = false;
				ui->lineEdit_2->setEchoMode(QLineEdit::Password);
				btn->setIcon(QIcon(":Login/img/eye2.png"));
			}

			});

		// 登录
		connect(ui->pushButton_3, &QPushButton::clicked, this, [=]() -> void {
			
			});

		// 注册
		connect(ui->pushButton_4, &QPushButton::clicked, this, [=]() -> void {
			return;
			});
	}

	// 设置阴影
	{
		QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
		shadow->setOffset(1, 1);
		//设置阴影颜色
		shadow->setColor(QColor("#000000"));
		//设置阴影圆角
		shadow->setBlurRadius(5);
		//给嵌套QWidget设置阴影
		ui->frame->setGraphicsEffect(shadow);
	}

	// 毛玻璃效果
	/*QGraphicsBlurEffect* effect = new QGraphicsBlurEffect(this);
	effect->setBlurRadius(5);
	ui->frame->setGraphicsEffect(effect);*/
}

Login::~Login()
{
    delete ui;
}

void Login::mousePressEvent(QMouseEvent* event)
{
	//界面移动
	int num = (event->globalPos() - this->pos()).y();
	if (event->button() == Qt::LeftButton && num < 50 && num > 10)
	{
		_flag = true;
		_position = std::move(event->globalPos() - this->pos());
		event->accept();
	}
}

void Login::mouseMoveEvent(QMouseEvent* event)
{
	//界面移动
	if (_flag)
	{
		this->move(event->globalPos() - _position);
		event->accept();
	}
}

void Login::mouseReleaseEvent(QMouseEvent* event)
{
	//界面移动
	_flag = false;
}