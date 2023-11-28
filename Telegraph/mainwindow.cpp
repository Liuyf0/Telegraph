#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    thread = new QThread(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginPushButton_clicked()
{
    QString password;
    QString username;
    username= ui->userLineEdit->text();
    password= ui->pwdLineEdit->text();
    if(username.isEmpty() || password.isEmpty()){
        QMessageBox::information(this, "提示", "用户名或密码不能为空");
        ui->userLineEdit->setFocus();
        return;
    }

    this->setEnabled(false);        // 是所有控件失效
    ui->progressBar->show();

    loginEvent = new LoginThread();
    loginEvent->username=username;
    loginEvent->password=password;
    loginEvent->moveToThread(thread);
    connect(this,&MainWindow::startThread,loginEvent,&LoginThread::startConnect);
    connect(loginEvent,&LoginThread::loginFailed,this,&MainWindow::loginFailed);
    connect(loginEvent,&LoginThread::loginSuccess,this,&MainWindow::loginSuccess);
    emit startThread();             // emit 用于触发自定义的信号
    thread->start();
}

void MainWindow::loginFailed(){
    QMessageBox::critical(this, "错误", loginEvent->errMsg);
    this->setEnabled(true);
    ui->progressBar->hide();
    thread->quit();
    thread->wait();
    delete loginEvent;
}

void MainWindow::loginSuccess(){
    QString md5;
    QMessageBox::information(this, "正确","登录成功");
    QString token;

    // 获取用户输入的密码并计算其 MD5 哈希值
    QByteArray bytePwd = ui->pwdLineEdit->text().toLatin1();
    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    md5 = bytePwdMd5.toHex();

    // 将密码的 MD5 哈希值与用户名拼接，并再次计算 MD5 哈希值作为令牌
    QString sign = md5 + ui->userLineEdit->text();
    bytePwd = sign.toLatin1();
    bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    token = bytePwdMd5.toHex();

//    home = new Home;
//    home->Token = token;
//    home->userName = ui->userLineEdit->text();
//    home->show();
//    home->startListen();
    this->hide();
}
