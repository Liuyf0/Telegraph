#include "loginthread.h"

LoginThread::LoginThread(QObject *parent) : QObject(parent)
{

}

void LoginThread::startConnect(){
    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected, this,&LoginThread::socket_Connected,Qt::DirectConnection);
    connect(socket, &QWebSocket::disconnected, this, &LoginThread::socket_Disconnected,Qt::DirectConnection);
    socket->open(QUrl("ws://localhost:8080"));
    // qDebug() << "Connected" << QThread::currentThreadId();
}

void LoginThread::socket_Connected(){

//    QString md5;
//    QByteArray bytePwd = password.toLatin1();
//    QByteArray bytePwdMd5 = QCryptographicHash::hash(bytePwd, QCryptographicHash::Md5);
//    md5.append(bytePwdMd5.toHex());
    connect(socket, &QWebSocket::textMessageReceived,
            this, &LoginThread::socket_Read_Data,Qt::DirectConnection);

    //生成 JSON 对象
    QJsonObject newMessage;
    QJsonObject msgData;
    newMessage.insert("action","login");
    msgData.insert("username",username);
    //msgData.insert("password",md5);
    msgData.insert("password",password);
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

void LoginThread::socket_Read_Data(QString buffer){
    if(!buffer.isEmpty())
    {
        QJsonParseError jsonError;
        QJsonDocument doucment = QJsonDocument::fromJson(buffer.toUtf8(), &jsonError);  // 转化为 JSON 文档
        // 解析未发生错误
        if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            // JSON 文档为对象
            if (doucment.isObject()) {
                QJsonObject object = doucment.object();  // 转化为对象
                if (object.contains("error")) {  // 包含指定的 key
                    QJsonValue value = object.value("error");
                    // 登录成功的处理
                    socket->close();
                    emit loginSuccess();
                }else{
                    // 登录失败的处理
                    errMsg = object.value("msg").toString();
                    socket->close();
                    emit loginFailed();
                }
            }
        }
    }
}


void LoginThread::socket_Disconnected(){

}
