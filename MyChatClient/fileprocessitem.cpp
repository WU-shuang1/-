#include "fileprocessitem.h"
#include "ui_fileprocessitem.h"
#include <QMessageBox>
#include "packDef.h"

FileProcessItem::FileProcessItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileProcessItem)
{
    ui->setupUi(this);
    flag = sendfile_continue;
}

FileProcessItem::~FileProcessItem()
{
    delete ui;
}

void FileProcessItem::setInfo(QString name, QString fileId, bool kind)
{
    m_fileId = fileId;
    QString houzhui = "";
    if(kind == filekind_recv)  {
        houzhui = "（接收）";
        flag = sendfile_pause;
        ui->pb_control->setText("开始");
        //隐藏取消按键
        ui->pb_cancel->setVisible(false);
    }
    else {
        houzhui = "（发送）";
        ui->pb_control->setText("暂停");
    }
    ui->lb_name->setText(name + houzhui);
    ui->progressBar->setRange(0, 100);

}

void FileProcessItem::updataBarValue(int value)
{
    ui->progressBar->setValue(value);
    if(value == 100) {
        ui->pb_control->setText("关闭");
        flag = sendfile_cancel;
    }
}

void FileProcessItem::on_pb_control_clicked()
{
    if(flag == sendfile_pause) {
        ui->pb_control->setText("暂停");
        flag = sendfile_continue;
    }
    else if(flag == sendfile_continue){
        ui->pb_control->setText("继续");
        flag = sendfile_pause;
    }else if(flag == sendfile_cancel) {
        //发送完毕关闭进度条
        Q_EMIT SIG_close(m_fileId);
        return;
    }

    //暂停、继续接收/发送文件
    Q_EMIT SIG_control(m_fileId, flag);
}


void FileProcessItem::on_pb_cancel_clicked()
{
    //取消接收/发送文件
    Q_EMIT SIG_cancel(m_fileId);
}

