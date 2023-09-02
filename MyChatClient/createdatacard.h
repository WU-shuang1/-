#ifndef CREATEDATACARD_H
#define CREATEDATACARD_H

#include <QWidget>
#include"groupdatacard.h"

namespace Ui {
class CreateDataCard;
}

class CreateDataCard : public GroupDataCard
{
    Q_OBJECT

public:
    explicit CreateDataCard(QWidget *parent = nullptr);
    ~CreateDataCard();
public:
    //设置群信息
    void setGroupInfo(QString name, int groupId, int iconId, int total);
signals:
    //更新群资料的信号
    void SIG_updataGroupInfo(QString name, int groupId, int iconId);
    //解散群聊的信号
    void SIG_deleteGroup(int groupId);
private slots:
    void on_pb_deleteGroup_clicked();

    void on_pb_save_clicked();

    void on_pb_close_clicked();

    void on_pb_prev_clicked();

    void on_pb_next_clicked();

private:
    Ui::CreateDataCard *ui;
    int m_tempIconId;
};

#endif // CREATEDATACARD_H
