/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <transformabstract.h>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit InfoDialog(TransformAbstract *transform, QWidget *parent = 0);
        ~InfoDialog();
        TransformAbstract * getTransform();
        
    private:
        Ui::InfoDialog *ui;
        TransformAbstract * transform;
};

#endif // INFODIALOG_H
