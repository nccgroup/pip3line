/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CUTWIDGET_H
#define CUTWIDGET_H

#include <QWidget>
#include "../cut.h"

namespace Ui {
class CutWidget;
}

class CutWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit CutWidget(Cut *ntransform, QWidget *parent = 0);
        ~CutWidget();
    private slots:
        void onFromChange(int value);
        void onLengthChange(int value);
        void onEverythingChange(bool);
    private:
        Ui::CutWidget *ui;
        Cut *transform;
};

#endif // CUTWIDGET_H
