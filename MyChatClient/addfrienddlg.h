#ifndef ADDFRIENDDLG_H
#define ADDFRIENDDLG_H

#include <QWidget>

namespace Ui {
class AddFriendDlg;
}

class AddFriendDlg : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriendDlg(QWidget *parent = nullptr);
    ~AddFriendDlg();

private:
    Ui::AddFriendDlg *ui;
};

#endif // ADDFRIENDDLG_H
