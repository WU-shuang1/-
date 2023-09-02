#ifndef FRIENDDATACARD_H
#define FRIENDDATACARD_H

#include <QWidget>

namespace Ui {
class FriendDataCard;
}

class FriendDataCard : public QWidget
{
    Q_OBJECT

public:
    explicit FriendDataCard(QWidget *parent = nullptr);
    ~FriendDataCard();
public:
    //设置好友信息
    void setFriendInfo(QString name, QString feeling, int icon, int id);
signals:
    //发送删除好友信号
    void SIG_deleteFriend(int id);
private slots:
    void on_pb_deleteFriend_clicked();

    void on_pb_close_clicked();

private:
    Ui::FriendDataCard *ui;
    int m_id;
};

#endif // FRIENDDATACARD_H
