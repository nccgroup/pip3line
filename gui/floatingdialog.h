/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef FLOATINGDIALOG_H
#define FLOATINGDIALOG_H

#include <QDialog>

namespace Ui {
class FloatingDialog;
}

class FloatingDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit FloatingDialog(QWidget *widget, QWidget *parent = 0);
        ~FloatingDialog();
    public slots:
        void closeEvent(QCloseEvent * event);
        
    private:
        Q_DISABLE_COPY(FloatingDialog)
        Ui::FloatingDialog *ui;
};

#endif // FLOATINGDIALOG_H
