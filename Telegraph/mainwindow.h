#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "loginthread.h"
#include <QThread>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QCryptographicHash>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginPushButton_clicked();

private:
    Ui::MainWindow *ui;
    QThread *thread;
    LoginThread* loginEvent;
    void loginFailed();
    void loginSuccess();

signals:
    void startThread();
};
#endif // MAINWINDOW_H
