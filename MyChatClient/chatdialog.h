#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QWidget>
#include <QMap>
#include <QVBoxLayout>
#include "fileprocessitem.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
signals:
    //把用户输入的内容通过信号发送给Kernel
    void SIG_sendMsg(QString content, int id);
    //发送文件信号
    void SIG_sendFile(QString FileName, int id);
    //取消接收/发送文件
    void SIG_cancel(QString fileId);
    //暂停、继续发送/接收文件
    void SIG_control(QString fileId, bool flag);
private slots:
    void on_pb_send_clicked();
    void on_pb_tool1_clicked();

    //取消接收/发送文件
    void slot_cancel(QString fileId);
    //发送完毕关闭进度条
    void slot_close(QString fileId);
    //暂停、继续接收/发送文件
    void slot_control(QString fileId, bool flag);
public:
    //设置窗口内容
    void setWindowInfo(QString name, int id);
    //设置发送来的聊天内容到界面
    void setChatMsg(QString content);
    //显示好友不在线
    void setFriendOffline();
    //添加进度条
    void addProcessBar(QString name, QString fileId, bool kind);
    //修改进度条
    void updataProcessBar(QString fileId, int value);
private:
    Ui::ChatDialog *ui;
    QVBoxLayout* m_playout;   //窗口管理器 垂直放置
    int m_id;
    QString m_name;
    QMap<QString, FileProcessItem*> m_fileIdToItem;   //文件上传/下载进度条
};

#endif // CHATDIALOG_H
