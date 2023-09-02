#ifndef FILEPROCESSITEM_H
#define FILEPROCESSITEM_H

#include <QWidget>

namespace Ui {
class FileProcessItem;
}

class FileProcessItem : public QWidget
{
    Q_OBJECT

public:
    explicit FileProcessItem(QWidget *parent = nullptr);
    ~FileProcessItem();
public:
    void setInfo(QString name, QString fileId, bool kind);
    void updataBarValue(int value);
signals:
    //取消接收/发送文件
    void SIG_cancel(QString fileId);
    //暂停、继续接收/发送文件
    void SIG_control(QString fileId, bool flag);
    //发送完毕关闭进度条
    void SIG_close(QString fileId);
private slots:
    void on_pb_control_clicked();

    void on_pb_cancel_clicked();

private:
    Ui::FileProcessItem *ui;
    int flag;
    QString m_fileId;
};

#endif // FILEPROCESSITEM_H
