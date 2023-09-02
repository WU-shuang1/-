#ifndef GROUPITEM_H
#define GROUPITEM_H

#include <QWidget>
#include "groupdatacard.h"

namespace Ui {
class GroupItem;
}

class GroupItem : public QWidget
{
    Q_OBJECT

public:
    explicit GroupItem(QWidget *parent = nullptr);
    ~GroupItem();
public:
    //设置群信息
    void setGroupInfo(QString name, int iconId, int total, int groupId);

signals:
    //发送打开聊天窗口的信号
    void SIG_clickGroupCard(int groupId);
private slots:
    void on_pb_icon_clicked();

    void on_pb_card_clicked();
public:
    GroupDataCard* m_pDataCard;
private:
    Ui::GroupItem *ui;
    QString m_name;
    int m_iconId;
    int m_groupId;
};

#endif // GROUPITEM_H
