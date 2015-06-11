/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef REGEXPHELPDIALOG_H
#define REGEXPHELPDIALOG_H

#include "appdialog.h"

namespace Ui {
class RegExpHelpDialog;
}

class RegExpHelpDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit RegExpHelpDialog(GuiHelper *nguiHelper, QWidget *parent = 0);
        ~RegExpHelpDialog();
        
    private:
        Q_DISABLE_COPY(RegExpHelpDialog)
        Ui::RegExpHelpDialog *ui;
};

#endif // REGEXPHELPDIALOG_H
