#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>
#include "frienddatacard.h"

namespace Ui {
class UserItem;
}

class UserItem : public QWidget
{
    Q_OBJECT

public:
    explicit UserItem(QWidget *parent = nullptr);
    ~UserItem();
signals:
    //点击好友卡片
    void SIG_clickCard(int id);

public:
    //设置卡片状态
    void setInfo(int id, int icon, int status, QString name, QString feeling);
    //设置下线状态
    void setOffline();
private slots:
    void on_pb_icon_clicked();

    void on_pb_card_clicked();

private:
    Ui::UserItem *ui;

public:
    //该卡片的用户信息
    FriendDataCard* m_pFriendCard;
    int m_userId;
    int m_iconId;
    int m_status;
    QString m_name;
    QString m_feeling;
};

#endif // USERITEM_H
