/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "appdialog.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit AboutDialog(GuiHelper * guiHelper, QWidget *parent = 0);
        ~AboutDialog();
        
    private:
        Q_DISABLE_COPY(AboutDialog)
        Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
