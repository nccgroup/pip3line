/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include "appdialog.h"

namespace Ui {
class InfoDialog;
}

class TransformAbstract;

class InfoDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit InfoDialog(GuiHelper * guiHelper, TransformAbstract *transform, QWidget *parent = 0);
        ~InfoDialog();
        TransformAbstract * getTransform();
        
    private:
        Q_DISABLE_COPY(InfoDialog)
        Ui::InfoDialog *ui;
        TransformAbstract * transform;
};

#endif // INFODIALOG_H
