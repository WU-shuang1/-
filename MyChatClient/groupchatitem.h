#ifndef GROUPCHATITEM_H
#define GROUPCHATITEM_H

#include <QWidget>

namespace Ui {
class GroupChatItem;
}

class GroupChatItem : public QWidget
{
    Q_OBJECT

public:
    explicit GroupChatItem(QWidget *parent = nullptr);
    ~GroupChatItem();
public:
    //设置成员属性
    void setMemberInfo(QString name, int id, int iconId, int status);
    //设置下线
    void setOffline();
public:
    QString m_name;
private:
    Ui::GroupChatItem *ui;
    int m_iconId;
    int m_id;
};

#endif // GROUPCHATITEM_H
