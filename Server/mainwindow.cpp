#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>
#include <QJsonArray>
#include <QFile>

#pragma execution_character_set("utf-8")
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Telegraph Server");
    ui->portLineEdit->setText("8080");
    ui->portLineEdit->setFocus();
}

void MainWindow::server_New_Connect(){
    QWebSocket* socketCon = server->nextPendingConnection();        // accept
    connect(socketCon, &QWebSocket::textMessageReceived, this, &MainWindow::socket_Read_Data);
    connect(socketCon, &QWebSocket::binaryMessageReceived, this, &MainWindow::socket_Read_File);
    connect(socketCon, &QWebSocket::disconnected, this, &MainWindow::socket_Disconnected);
    connectionList.append(socketCon);
    Log("A Client connect!");
}

void MainWindow::socket_Read_File(QByteArray message){
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QHash<QString, User>::const_iterator i;
    QString senderName;
    Log("write");
   for(i=onlineList.constBegin(); i!=onlineList.constEnd(); ++i){
        if(i.value().socket == pClient){
            senderName = i.key();
            break;
        }
    }
    if(line.contains(senderName)){
        if(message==QString("ok,finished52121225").toUtf8()){
             onlineList[senderName].file->close();
             if(line[senderName].value("to").toString().contains("[群聊]")){
                 QList<QString> users = groupList[line[senderName].value("to").toString().replace("[群聊]","")];
                 QListIterator<QString> ii(users);
                 while (ii.hasNext()) {
                     QString user = ii.next();
                     if(onlineList.contains(user)){
                         if(user==line[senderName].value("from").toString()){
                             continue;
                         }
                          onlineList[user].sendFile(onlineList[senderName].file->fileName(),line[senderName].value("filename").toString(),line[senderName].value("to").toString(),line[senderName].value("size").toInt(),line[senderName].value("mid").toString(),line[senderName].value("from").toString());
                     }
                 }
             }else{
                QHash<QString, User>::iterator i = onlineList.find(line[senderName].value("to").toString());
                if(i != onlineList.end()){
                    Log("writeFound");

                 i.value().sendFile(onlineList[senderName].file->fileName(),line[senderName].value("filename").toString(),line[senderName].value("from").toString(),line[senderName].value("size").toInt(),line[senderName].value("mid").toString());

                }else{
                    onlineList[line[senderName].value("from").toString()].saveFileMsg(line[senderName]);
                }
            }
            delete onlineList[senderName].file;
        }else{
            onlineList[senderName].file->write(message);
        }
    }
}

void MainWindow::socket_Read_Data(QString message)
{
    // 获取发出信号的 WebSocket 客户端
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

    // 检查接收到的消息是否非空
    if (!message.isEmpty())
    {
        // 如果消息长度小于或等于 5000 字符，记录消息
        if (message.length() <= 5000)
        {
            Log(message);
        }

        // 处理接收到的消息
        handleReq(message, pClient);
    }

    // 准备一个响应消息
    QJsonObject newMessage;

    // 检查是否有错误消息需要包含在响应中
    if (!errorMsg.isEmpty())
    {
        newMessage.insert("error", 1);
        newMessage.insert("msg", errorMsg);
        errorMsg = "";
    }
    else
    {
        newMessage.insert("error", 0);
    }

    // 检查是否有要发送的回复（wiiRepaly == 0 表示不发送回复）
    if (wiiRepaly == 0)
    {
        // 不发送回复
    }
    else
    {
        // 准备并发送一个 JSON 格式的响应给客户端
        QJsonDocument document;
        document.setObject(newMessage);
        QByteArray byteArray = document.toJson(QJsonDocument::Compact);
        QString strJson(byteArray);
        pClient->sendTextMessage(strJson.toUtf8());
    }

    // 将 wiiRepaly 设置为 1，表示已发送回复
    wiiRepaly = 1;
}

void MainWindow::handleReq(QString data,QWebSocket *pClient){
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(data.toUtf8(), &jsonError);  // 转化为 JSON 文档
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (doucment.isObject()) { // JSON 文档为对象
            QJsonObject object = doucment.object();  // 转化为对象
            if (object.contains("action")) {  // 包含指定的 key
                QJsonValue value = object.value("action");
                QJsonObject obj = object.value("data").toObject();
                if(value.toString()=="register"){
                    registerNewUser(obj);
                    wiiRepaly=1;
                }else if(value.toString()=="login"){
                    Login(obj);
                    wiiRepaly=1;
                }else if(value.toString()=="auth"){
                    addNewUser(obj,pClient);
                    wiiRepaly=0;
                }else if(value.toString()=="sendTextMsg"){
                    sendMsg(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="sendImg"){
                    sendImg(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="sendFile"){
                    sendFile(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="download"){
                    Download(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="createGroup"){
                    createGroup(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="changeProfile"){
                    changeProfile(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="withDraw"){
                    withDraw(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="addGrade"){
                    addGrade(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="decreaseGrade"){
                    onlineList[obj.value("from").toString()].decreaseGrade(obj.value("grade").toInt());
                    wiiRepaly=0;
                }else if(value.toString()=="sendShanke"){
                    if(onlineList.contains(obj.value("to").toString())){
                        onlineList[obj.value("to").toString()].sendShake(obj.value("from").toString());
                    }
                    wiiRepaly=0;
                }else if(value.toString()=="sendRedPack"){
                    sendRedPack(obj);
                    wiiRepaly=0;
                }else if(value.toString()=="garbRedPack"){
                    onlineList[obj.value("from").toString()].garbRedPack(obj.value("mid").toString());
                    wiiRepaly=0;
                }else if(value.toString()=="sendSpeak"){
                    sendSpeak(obj);
                    wiiRepaly=0;
                }
            }
        }
    }else{
        errorMsg = "无法解析请求";
    }
}

void MainWindow::sendSpeak(QJsonObject msgData){
    if(msgData.value("to").toString().contains("[群聊]")){
        QList<QString> users = groupList[msgData.value("to").toString().replace("[群聊]","")];
        QListIterator<QString> i(users);
        while (i.hasNext()) {
            QString user = i.next();
            if(onlineList.contains(user)){
                if(user==msgData.value("from").toString()){
                    continue;
                }
                onlineList[user].sendSpeak(msgData.value("sec").toInt(),msgData.value("body").toString(),msgData.value("to").toString(),msgData.value("mid").toString(),msgData.value("from").toString());
            }
        }
        return;
    }
    QHash<QString, User>::iterator i = onlineList.find(msgData.value("to").toString());
    if(i != onlineList.end()){
        i.value().sendSpeak(msgData.value("sec").toInt(),msgData.value("body").toString(),msgData.value("from").toString(),msgData.value("mid").toString());
    }else{
       //onlineList[msgData.value("from").toString()].saveSpeakMsg(msgData);
    }
}

void MainWindow::sendRedPack(QJsonObject msgData){
    onlineList[msgData.value("from").toString()].initRedPack(msgData);
    if(msgData.value("to").toString().contains("[群聊]")){
        QList<QString> users = groupList[msgData.value("to").toString().replace("[群聊]","")];
        QListIterator<QString> i(users);
        while (i.hasNext()) {
            QString user = i.next();
            if(onlineList.contains(user)){
                if(user==msgData.value("from").toString()){
                    continue;
                }
                onlineList[user].sendRedPack(msgData.value("mid").toString(),msgData.value("to").toString(),msgData.value("from").toString());
            }
        }
        return;
    }
    QHash<QString, User>::iterator i = onlineList.find(msgData.value("to").toString());
    if(i != onlineList.end()){
        i.value().sendRedPack(msgData.value("mid").toString(),msgData.value("from").toString());
    }else{
        onlineList[msgData.value("from").toString()].saveRedPackMsg(msgData);
    }
}

void MainWindow::addGrade(QJsonObject msgData){
    onlineList[msgData.value("from").toString()].addGrade(msgData.value("grade").toInt());
}

void MainWindow::withDraw(QJsonObject msgData){
    if(msgData.value("to").toString().contains("[群聊]")){
        QList<QString> users = groupList[msgData.value("to").toString().replace("[群聊]","")];
        QListIterator<QString> i(users);
        while (i.hasNext()) {
            QString user = i.next();
            if(onlineList.contains(user)){
                if(user==msgData.value("from").toString()){
                    continue;
                }
                onlineList[user].withDraw(msgData.value("mid").toString(),msgData.value("to").toString());
            }
        }
        return;
    }
    QHash<QString, User>::iterator i = onlineList.find(msgData.value("to").toString());
    if(i != onlineList.end()){
        i.value().withDraw(msgData.value("mid").toString(),msgData.value("from").toString());
    }else{
        if(onlineList.contains(msgData.value("from").toString())){
            onlineList[msgData.value("from").toString()].withDrawOfflineMsg(msgData.value("mid").toString());
        }

    }
}

void MainWindow::changeProfile(QJsonObject msgData){
    onlineList[msgData.value("origin").toString()].changeProfile(msgData);
    onlineList[msgData.value("origin").toString()].socket->close();
}

void MainWindow::createGroup(QJsonObject msgData){
   QJsonArray array = msgData.value("user").toArray();
   QList<QString> users;
   int nSize = array.size();
   for (int i = 0; i < nSize; ++i) {
        users.append(array.at(i).toString());
   }
   groupList.insert(msgData.value("groupname").toString(),users);
   for (int i = 0; i < nSize; ++i) {
        //users.append(array.at(i).toString());
       if(onlineList.contains(array.at(i).toString())){
            onlineList[array.at(i).toString()].newGroup(msgData.value("groupname").toString());
       }
   }
}

void MainWindow::Download(QJsonObject msgData){
    onlineList[msgData.value("receiver").toString()].sendFileData(msgData.value("name").toString());
}

void MainWindow::sendFile(QJsonObject msgData){
    line.insert(msgData.value("from").toString(),msgData);
    onlineList[msgData.value("from").toString()].file = new QFile("tmp/"+QString::number(QDateTime::currentDateTime().toTime_t()));
    onlineList[msgData.value("from").toString()].file->open(QIODevice::WriteOnly);
}

void MainWindow::sendMsg(QJsonObject msgData){
    if(msgData.value("to").toString().contains("[群聊]")){
        QList<QString> users = groupList[msgData.value("to").toString().replace("[群聊]","")];
        QListIterator<QString> i(users);
        while (i.hasNext()) {
            QString user = i.next();
            if(onlineList.contains(user)){
                if(user==msgData.value("from").toString()){
                    continue;
                }
                onlineList[user].sendTextMsg(msgData.value("body").toString(),msgData.value("to").toString(),msgData.value("mid").toString(),msgData.value("from").toString());
            }
        }
        return;
    }
    QHash<QString, User>::iterator i = onlineList.find(msgData.value("to").toString());
    if(i != onlineList.end()){
        i.value().sendTextMsg(msgData.value("body").toString(),msgData.value("from").toString(),msgData.value("mid").toString());
    }else{
        onlineList[msgData.value("from").toString()].saveTextMsg(msgData);
    }
}

void MainWindow::sendImg(QJsonObject msgData){
    if(msgData.value("to").toString().contains("[群聊]")){
        QList<QString> users = groupList[msgData.value("to").toString().replace("[群聊]","")];
        QListIterator<QString> i(users);
        while (i.hasNext()) {
            QString user = i.next();
            if(onlineList.contains(user)){
                if(user==msgData.value("from").toString()){
                    continue;
                }
                onlineList[user].sendImg(msgData.value("body").toString(),msgData.value("to").toString(),msgData.value("mid").toString(),msgData.value("from").toString());
            }
        }
        return;
    }
    QHash<QString, User>::iterator i = onlineList.find(msgData.value("to").toString());
    if(i != onlineList.end()){
        i.value().sendImg(msgData.value("body").toString(),msgData.value("from").toString(),msgData.value("mid").toString());
    }else{
       onlineList[msgData.value("from").toString()].saveImgMsg(msgData);
    }
}

void MainWindow::registerNewUser(QJsonObject userData){
    User newUser;
    newUser.db = &db;
    newUser.setUserInfo(userData.value("username").toString(),userData.value("password").toString(),userData.value("sex").toString(),userData.value("email").toString());
    if(!newUser.Register()){
        errorMsg = newUser.errMsg;
    }else{
         Log("New User "+userData.value("username").toString() +" Registered.");
    }
}

void MainWindow::Login(QJsonObject userData){
    User newUser;
    newUser.db = &db;
    newUser.setUserInfo(userData.value("username").toString(),userData.value("password").toString());
    if(!newUser.Login()){
        errorMsg = newUser.errMsg;
    }else{
         Log("User："+userData.value("username").toString() +" Log in.");
    }
}

void MainWindow::addNewUser(QJsonObject userData,QWebSocket* pClient){
    User newUser;
    newUser.db = &db;
    if(!newUser.LoginByToken(userData.value("token").toString() )){
        Log(userData.value("token").toString()+" auth failed.");
        return;
    }
    newUser.socket = pClient;
    onlineList.insert(newUser.userName,newUser);
    //获取在线用户列表
    QHash<QString, User>::const_iterator i;
    QJsonArray onlineUser;
    for(i=onlineList.constBegin(); i!=onlineList.constEnd(); ++i){
        QJsonObject userInfo;
        userInfo.insert("uid",i.value().uid);
        userInfo.insert("username",i.value().userName);
        userInfo.insert("email",i.value().Email);
        userInfo.insert("avatar",i.value().Avatar);
        onlineUser.append(userInfo);
        onlineList[i.value().userName].sendBroadcast(1,newUser.userName);
    }
    onlineList[newUser.userName].Init(onlineUser);
}

void MainWindow::socket_Disconnected()
{
    // 从连接断开的客户端中获取用户名
    QString logOffUser;
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    // 遍历在线用户列表查找与断开连接的客户端匹配的用户名
    for (const auto &user : onlineList) {
        if (user.socket == pClient) {
            logOffUser = user.userName;
            break;
        }
    }
    // 记录断开连接的事件
    Log("Disconnected!");
    // 如果找到了匹配的用户名
    if(!logOffUser.isEmpty()){
        // 从在线用户列表中移除断开连接的用户
        onlineList.remove(logOffUser);
        // 记录用户下线事件
        Log("User " + logOffUser + " Log off!");
        // 向其他在线用户广播当前用户下线的消息
        for (const auto &user : onlineList) {
            onlineList[user.userName].sendBroadcast(0, logOffUser);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    server->close();
    qDeleteAll(connectionList.begin(), connectionList.end());
}

void MainWindow::Log(QString s){
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("[hh:mm:ss] ");
    ui->logTextEdit->setText(ui->logTextEdit->toPlainText()+str+s+"\n");
}

void MainWindow::on_onPushButton_clicked()
{
    // 关闭打开按钮
    ui->onPushButton->setEnabled(false);

    // 连接socket
    server = new QWebSocketServer(QStringLiteral("Echo Server"),QWebSocketServer::NonSecureMode, this);
    qint16 port = ui->portLineEdit->text().toInt();
    if(!server->listen(QHostAddress::Any, port)){
        Log("Failed to start WS server. Error: " + server->errorString());
        ui->onPushButton->setEnabled(true);
        return;

    }
    connect(server, &QWebSocketServer::newConnection,this, &MainWindow::server_New_Connect);
    Log("Listen succeessfully!(Port: "+ui->portLineEdit->text()+")");


    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");                            // 数据库ip
    db.setPort(3306);                                       // 数据库port
    db.setUserName("root");       							// 登陆MYSQL的用户名
    db.setPassword("root");                                 // 登陆的密码
    db.setDatabaseName("telegraph");  						// 登陆数据库的名称
    if(!db.open()){
        Log("Failed to connect to MySQL. Error: " + db.lastError().text());
        ui->onPushButton->setEnabled(true);
        return;
    }
    Log("Database succeessfully!(" + db.hostName() + ":" + QString::number(db.port()) + ")");
    ui->offPushButton->setEnabled(true);
}

void MainWindow::on_offPushButton_clicked()
{
    server->close();
    qDeleteAll(connectionList.begin(), connectionList.end());
    db.close();
    Log("Server stoped.");
    delete server;
    ui->onPushButton->setEnabled(true);
    ui->offPushButton->setEnabled(false);
}

void MainWindow::on_clearPushButton_clicked()
{
    ui->logTextEdit->setText("");
}
