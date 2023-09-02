#include "addfrienddlg.h"
#include "ui_addfrienddlg.h"

AddFriendDlg::AddFriendDlg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriendDlg)
{
    ui->setupUi(this);
}

AddFriendDlg::~AddFriendDlg()
{
    delete ui;
}
