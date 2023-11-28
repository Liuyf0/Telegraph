#ifndef LOGINTHREAD_H
#define LOGINTHREAD_H

#include <QObject>
#include <QWebSocket>
#include <QWebSocket>
#include <QByteArray>
#include <QCryptographicHash>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

class LoginThread : public QObject
{
    Q_OBJECT
public:
    explicit LoginThread(QObject *parent = nullptr);
    void socket_Read_Data(QString buffer);
    void socket_Disconnected();
    void socket_Connected();
    void startConnect();

    QString username;
    QString password;
    QString errMsg;

protected:
    void run();

signals:
    void loginSuccess();
    void loginFailed();

public slots:

private:
    QWebSocket *socket;
};

#endif // LOGINTHREAD_H
