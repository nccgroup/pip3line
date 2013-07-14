/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit AboutDialog(QWidget *parent = 0);
        ~AboutDialog();
        
    private:
        Q_DISABLE_COPY(AboutDialog)
        Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H