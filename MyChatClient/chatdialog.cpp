#include "chatdialog.h"
#include "ui_chatdialog.h"
#include <QTime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "packDef.h"
#include <QLabel>
ChatDialog::ChatDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatDialog)
{
    ui->setupUi(this);

    //初始化垂直布局的层
    m_playout = new QVBoxLayout;
    //设置与外控件上下左右的距离
    m_playout->setContentsMargins(0, 0, 0, 0);
    //设置这个层中每个控件彼此之间的距离
    m_playout->setSpacing(3);
    //把层设置到外部的控件上
    ui->wt_list->setLayout(m_playout);
}

ChatDialog::~ChatDialog()
{
    delete ui;
    if(m_playout) {
        delete m_playout;
        m_playout = nullptr;
    }

}

void ChatDialog::on_pb_send_clicked()
{
    // 1.获取用户输入的纯文本内容，校验内容
    QString content = ui->te_chat->toPlainText();
    if(content.isEmpty()) {
        return;
    }

    // 2.获取带格式的用户输入内容，清空编辑窗口
    content = ui->te_chat->toHtml();
    ui->te_chat->clear();

    if(content.size() > DEF_CONTENT_SIZE) {
        QMessageBox::about(this, "提示", "聊天内容过大，不能超过1000字");
        return;
    }
    // 3.把用户输入的内容设置到浏览控件上
    ui->tb_chat->append(QString("【我】%1").arg(QTime::currentTime().toString("hh:mm:ss")));
    ui->tb_chat->append(content);

    // 4.把用户输入的内容通过信号发送给Kernel
    Q_EMIT SIG_sendMsg(content, m_id);
}

void ChatDialog::setWindowInfo(QString name, int id)
{
    qDebug()<< __func__;
    //保存成员的值
    m_id = id;
    m_name = name;
    //设置聊天窗口的title
    setWindowTitle(QString("与【%1】的聊天").arg(name));
}
//设置发送来的聊天内容到界面
void ChatDialog::setChatMsg(QString content)
{
    qDebug()<< __func__;
    // 把收到的内容设置到浏览控件上
    ui->tb_chat->append(QString("【%1】%2").arg(m_name).arg(QTime::currentTime().toString("hh:mm:ss")));
    ui->tb_chat->append(content);
}
//显示好友不在线
void ChatDialog::setFriendOffline()
{
    qDebug()<< __func__;
    //显示好友不在线
    ui->tb_chat->append(QString("【%1】%2 %3").arg(m_name).arg(QTime::currentTime().toString("hh:mm:ss")).arg("好友不在线"));
}
//添加进度条
void ChatDialog::addProcessBar(QString name,QString fileId, bool kind)
{
    qDebug()<< __func__;
    FileProcessItem* pItem = new FileProcessItem;
    connect(pItem, SIGNAL(SIG_cancel(QString)), this, SLOT(slot_cancel(QString)));
    connect(pItem, SIGNAL(SIG_close(QString)), this, SLOT(slot_close(QString)));
    connect(pItem, SIGNAL(SIG_control(QString, bool)), this, SLOT(slot_control(QString, bool)));
    pItem->setInfo(name, fileId, kind);

    m_playout->addWidget(pItem);
    m_fileIdToItem[fileId] = pItem;
}
//修改进度条
void ChatDialog::updataProcessBar(QString fileId, int value)
{
    if(m_fileIdToItem.find(fileId) != m_fileIdToItem.end()) {
        m_fileIdToItem[fileId]->updataBarValue(value);
    }
}


void ChatDialog::on_pb_tool1_clicked()
{
    QString fileName =QFileDialog::getOpenFileName(this, "选择文件", "C:/",
                                                   "All Files (*);;Image Files (*.png *.jpg);;Text files (*.txt)");
    if(fileName.isEmpty()) return;

    Q_EMIT SIG_sendFile(fileName, m_id);
}
//取消接收/发送文件
void ChatDialog::slot_cancel(QString fileId)
{
    Q_EMIT SIG_cancel(fileId);
    //关闭进度条
    slot_close(fileId);
}
//发送完毕关闭进度条
void ChatDialog::slot_close(QString fileId)
{
    auto ite = m_fileIdToItem.find(fileId);
    if(*ite) {
        m_playout->removeWidget(*ite);
        delete (*ite);
        (*ite) = nullptr;
        m_fileIdToItem.erase(ite);
    }
}
//暂停、继续接收/发送文件
void ChatDialog::slot_control(QString fileId, bool flag)
{
    Q_EMIT SIG_control(fileId, flag);
}

