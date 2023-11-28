#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QCryptographicHash>

#include "ui_mainwindow.h"
#include "loginthread.h"
#include "regwindow.h"

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

    void on_regPushButton_clicked();

private:
    Ui::MainWindow *ui;
    QThread *thread;
    LoginThread* loginEvent;
    RegWindow *reg;
    void loginFailed();
    void loginSuccess();

signals:
    void startThread();
};
#endif // MAINWINDOW_H
