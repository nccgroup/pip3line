/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CUSTOMDIALOGDOCKWIDGET_H
#define CUSTOMDIALOGDOCKWIDGET_H

#include <QDockWidget>

namespace Ui {
class CustomDialogDockWidget;
}

class CustomDialogDockWidget : public QDockWidget
{
        Q_OBJECT
        
    public:
        explicit CustomDialogDockWidget(const QString &title, QWidget *parent = 0, QWidget *item = 0);
        ~CustomDialogDockWidget();
        void addWidget(QWidget *item);
        
    private:
        Ui::CustomDialogDockWidget *ui;
};

#endif // CUSTOMDIALOGDOCKWIDGET_H
