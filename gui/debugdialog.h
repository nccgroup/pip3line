/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <QDialog>

namespace Ui {
class DebugDialog;
}

class DebugDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit DebugDialog(QWidget *parent = 0);
        ~DebugDialog();
        
        void closeEvent(QCloseEvent *event);
    private slots:
        void on_loadPushButton_clicked();
    private:
        Ui::DebugDialog *ui;
};

#endif // DEBUGDIALOG_H
