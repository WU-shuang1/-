#ifndef USERDATACARD_H
#define USERDATACARD_H

#include <QWidget>

namespace Ui {
class UserDataCard;
}

class UserDataCard : public QWidget
{
    Q_OBJECT

public:
    explicit UserDataCard(QWidget *parent = nullptr);
    ~UserDataCard();
signals:
    //发送用户信息的信号
    void SIG_saveUserData(QString name, QString feeling, int iconId);
public:
    //设置用户信息
    void SetUserdata(QString name, QString feeling, int iconId);
private slots:
    void on_pb_prev_clicked();

    void on_pb_next_clicked();

    void on_pb_close_clicked();

    void on_pb_save_clicked();

private:
    Ui::UserDataCard *ui;
    QString m_name;
    QString m_feeling;
    int m_iconId;
    int m_tempIconId;
};

#endif // USERDATACARD_H
