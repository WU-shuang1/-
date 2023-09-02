#include "groupchatdialog.h"
#include "ui_groupchatdialog.h"
#include <QTime>
#include <QFileDialog>
#include <QMessageBox>
#include "packDef.h"
GroupChatDialog::GroupChatDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupChatDialog)
{
    ui->setupUi(this);
    m_onlineMemberNum = 0;
    //初始化垂直布局的层
    m_playout = new QVBoxLayout;
    //设置与外控件上下左右的距离
    m_playout->setContentsMargins(0, 0, 0, 0);
    //设置这个层中每个控件彼此之间的距离
    m_playout->setSpacing(3);
    //把层设置到外部的控件上
    ui->wt_list->setLayout(m_playout);
}

GroupChatDialog::~GroupChatDialog()
{
    delete ui;
    if(m_playout) {
        delete m_playout;
        m_playout = nullptr;
    }
    auto ite = m_idToItemMap.begin();
    while(ite != m_idToItemMap.end()) {
        if(*ite) {
            delete *ite;
            (*ite) = nullptr;
            ite = m_idToItemMap.erase(ite);
        }else ite++;
    }
    m_idToItemMap.clear();
}
//设置基本信息
void GroupChatDialog::setInfo(QString groupName, int groupId, int total)
{
    m_totalNum = total;
    setWindowInfo(groupName, groupId);
    ui->lb_member->setText(QString("群成员（%1/%2）").arg(m_onlineMemberNum).arg(m_totalNum));
    repaint();
}
//设置窗口内容
void GroupChatDialog::setWindowInfo(QString groupName, int groupId)
{
    //保存成员的值
    m_groupId = groupId;
    m_groupName = groupName;
    //设置聊天窗口的title
    setWindowTitle(QString("【%1】多人聊天").arg(m_groupName));
}
//添加群成员
void GroupChatDialog::addMember(GroupChatItem *item, int userId)
{
    m_idToItemMap[userId] = item;
    m_playout->addWidget(item);
}
//删除群成员
void GroupChatDialog::deleteMember(int userId)
{
    //回收该成员的item空间
    auto ite = m_idToItemMap.find(userId);
    if(ite != m_idToItemMap.end()) {
        m_playout->removeWidget(*ite);
        if((*ite)) {
            delete (*ite);
            (*ite) = nullptr;
            m_idToItemMap.erase(ite);
        }
        //将总人数和在线人数-1 并重新绘制
        m_onlineMemberNum--;
        m_totalNum--;
        ui->lb_member->setText(QString("群成员（%1/%2）").arg(m_onlineMemberNum).arg(m_totalNum));
        repaint();
    }
}
//增加在线人数
void GroupChatDialog::addOnlineMember()
{
    m_onlineMemberNum++;
    ui->lb_member->setText(QString("群成员（%1/%2）").arg(m_onlineMemberNum).arg(m_totalNum));
    repaint();
}
//减少在线人数
void GroupChatDialog::deleteOnlineMember()
{
    m_onlineMemberNum--;
    ui->lb_member->setText(QString("群成员（%1/%2）").arg(m_onlineMemberNum).arg(m_totalNum));
    repaint();
}
//设置发送来的聊天内容到界面
void GroupChatDialog::setChatMsg(QString content, int memberId)
{
    // 把收到的内容设置到浏览控件上
    ui->tb_chat->append(QString("【%1】%2").arg(m_idToItemMap[memberId]->m_name).arg(QTime::currentTime().toString("hh:mm:ss")));
    ui->tb_chat->append(content);
}

void GroupChatDialog::on_pb_send_clicked()
{
    // 1.获取用户输入的纯文本内容，校验内容
    QString content = ui->te_chat->toPlainText();
    if(content.isEmpty()) {
        return;
    }

    if(content.size() > DEF_CONTENT_SIZE) {
        QMessageBox::about(this, "提示", "聊天内容过大，不能超过1000字");
        return;
    }

    // 2.获取带格式的用户输入内容，清空编辑窗口
    content = ui->te_chat->toHtml();
    ui->te_chat->clear();

    // 3.把用户输入的内容设置到浏览控件上
    ui->tb_chat->append(QString("【我】%1").arg(QTime::currentTime().toString("hh:mm:ss")));
    ui->tb_chat->append(content);


    // 4.把用户输入的内容通过信号发送给Kernel
    Q_EMIT SIG_sendGroupMsg(content, m_groupId);
}


void GroupChatDialog::on_pb_tool1_clicked()
{
    QString fileName =QFileDialog::getOpenFileName(this, "选择文件", "C:/",
                                                   "All Files (*);;Image Files (*.png *.jpg);;Text files (*.txt)");
    if(fileName.isEmpty()) return;
    Q_EMIT SIG_sendGroupFile(fileName, m_groupId);
}

