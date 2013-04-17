/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ROTXWIDGET_H
#define ROTXWIDGET_H

#include <QWidget>
#include "../rotx.h"

namespace Ui {
class RotXWidget;
}

class RotXWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit RotXWidget(Rotx *ntransform, QWidget *parent = 0);
        ~RotXWidget();
    private slots:
        void onUpdateType(int val);
    private:
        Ui::RotXWidget *ui;
        Rotx *transform;
};

#endif // ROTXWIDGET_H
