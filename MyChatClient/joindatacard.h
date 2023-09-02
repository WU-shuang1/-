#ifndef JOINDATACARD_H
#define JOINDATACARD_H

#include <QWidget>
#include "groupdatacard.h"

namespace Ui {
class JoinDataCard;
}

class JoinDataCard : public GroupDataCard
{
    Q_OBJECT

public:
    explicit JoinDataCard(QWidget *parent = nullptr);
    ~JoinDataCard();
public:
    //设置群信息
    virtual void setGroupInfo(QString name, int groupId, int iconId, int total);
signals:
    //发送退出群聊的信号给kernel
    void SIG_exitGroup(int groupId);
private slots:
    void on_pb_close_clicked();

    void on_pb_exitGroup_clicked();

private:
    Ui::JoinDataCard *ui;
};

#endif // JOINDATACARD_H
