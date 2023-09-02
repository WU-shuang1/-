#ifndef GROUPDATACARD_H
#define GROUPDATACARD_H

#include <QWidget>

class GroupDataCard : public QWidget
{
    Q_OBJECT
public:
    explicit GroupDataCard(QWidget *parent = nullptr);

public:
    //设置群信息
    virtual void setGroupInfo(QString name, int groupId, int iconId, int total)=0;
signals:

public:
    QString m_name;
    int m_groupId;
    int m_iconId;
    int m_total;
};

#endif // GROUPDATACARD_H
