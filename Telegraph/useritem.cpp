#include "useritem.h"
#include "ui_useritem.h"

#pragma execution_character_set("utf-8")

UserItem::UserItem(QWidget *parent) : QWidget(parent), ui(new Ui::UserItem)
{
    ui->setupUi(this);
    ui->unreadLabel->hide();
}

void UserItem::setUserInfo(QString username, QString des, QString avatar){
    ui->userNameLabel->setText(username);
    userName = username;
    ui->onOrOffLabel->setText(des);
    avatarId = avatar;
    /* 将一个图像作为 label 的边框，并设置这个边框具有圆角。
     * avatarId 是一个字符串，代表了图像的路径，
     * 可能包含一个末尾的 "b"（该部分被替换为空字符串）。
     * 这样的操作通常用于显示用户的头像。
     */
    ui->avatarLabel->setStyleSheet("border-image: url(avatars/"+avatar+".jpg);border-radius:32;");
}

void UserItem::addUnreadMessage(){
    ui->unreadLabel->show();
    unread++;
    ui->unreadLabel->setText(QString::number(unread));
    ui->unreadLabel->adjustSize();
}

void UserItem::setStatus(int status){
    if(status==1){
         ui->avatarLabel->setStyleSheet("border-image: url(avatars/"+avatarId.replace("b","")+".jpg);border-radius:32;");
         ui->onOrOffLabel->setText("在线");
    }else{
        ui->avatarLabel->setStyleSheet("border-image: url(avatars/"+avatarId.replace("b","")+"b.jpg);border-radius:32;");
        ui->onOrOffLabel->setText("离线");
    }
}

void UserItem::readAll(){
    unread=0;
    ui->unreadLabel->hide();

}

UserItem::~UserItem()
{
    delete ui;
}
