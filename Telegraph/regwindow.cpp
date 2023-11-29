#include "regwindow.h"
#include "ui_regwindow.h"

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

// #pragma execution_character_set("utf-8")
RegWindow::RegWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RegWindow)
{
    ui->setupUi(this);
    ui->sexcCmboBox->addItem("男");
    ui->sexcCmboBox->addItem("女");
//    ui->sexcCmboBox->setCurrentIndex(0);
//    ui->userLineEdit->setFocus();
}

RegWindow::~RegWindow()
{
    delete ui;
}

void RegWindow::socket_Read_Data(QString buffer){
    if(!buffer.isEmpty())
    {
        QJsonParseError jsonError;
        QJsonDocument doucment = QJsonDocument::fromJson(buffer.toUtf8(), &jsonError);  // 转化为 JSON 文档

        if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
            if (doucment.isObject()) { // JSON 文档为对象
                QJsonObject object = doucment.object();  // 转化为对象
                if (object.contains("error")) {  // 包含指定的 key
                    QJsonValue value = object.value("error");
                    if(!value.toInt()){
                       QMessageBox::information(this, "提示", tr("注册成功"));
                       socket->close();
                       this->close();
                    }else{
                       QMessageBox::critical(this, "错误", object.value("msg").toString());
                       socket->close();
                       ui->userLineEdit->clear();
                       ui->pwdLineEdit->clear();
                       ui->pwdpwdLineEdit->clear();
                       ui->sexcCmboBox->setCurrentIndex(0);
                       ui->emailLineEdit->clear();
                       ui->userLineEdit->setFocus();
                    }
                    ui->regPushButton->setEnabled(true);
                }
            }
        }
    }
}
void RegWindow::socket_Disconnected(){

}

void RegWindow::socket_Connected(){
    QString username = ui->userLineEdit->text();
    QString password = ui->pwdLineEdit->text();
    QString sex = ui->sexcCmboBox->currentText();
    QString email = ui->emailLineEdit->text();

    QString md5;
    QByteArray bytePwd = password.toLatin1();
    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
    md5.append(bytePwdMd5.toHex());

    connect(socket, &QWebSocket::textMessageReceived,
                this, &RegWindow::socket_Read_Data);

    //生成 JSON 对象
    QJsonObject newMessage;
    QJsonObject msgData;
    newMessage.insert("action","register");
    msgData.insert("username",username);
    msgData.insert("password",md5); // msgData.insert("password",md5);
    msgData.insert("sex",sex);
    msgData.insert("email",email);
    newMessage.insert("data",msgData);

    // 将 JSON 对象转换为 JSON 文档
    QJsonDocument document;
    document.setObject(newMessage);

    // 将 JSON 文档转换为紧凑格式的字符串表示
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);

    //发送数据
    socket->sendTextMessage(strJson.toUtf8());
    socket->flush();
}

void RegWindow::on_regPushButton_clicked()
{
    QString username = ui->userLineEdit->text();
    QString password = ui->pwdLineEdit->text();
    if(password != ui->pwdpwdLineEdit->text()){
        QMessageBox::warning(this, "错误", tr("两次密码输入不一致"));
        return;
    }
    if(username.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this, "错误", tr("用户名或密码不能为空"));
        return;
    }

    ui->regPushButton->setEnabled(false);

    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected, this,&RegWindow::socket_Connected);
    connect(socket, &QWebSocket::disconnected, this, &RegWindow::socket_Disconnected);
    socket->open(QUrl("ws://localhost:8080"));
}
