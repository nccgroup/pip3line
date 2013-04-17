/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef REVERSEWIDGET_H
#define REVERSEWIDGET_H

#include <QWidget>
#include "../reverse.h"

namespace Ui {
class ReverseWidget;
}

class ReverseWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ReverseWidget(Reverse *transform, QWidget *parent = 0);
        ~ReverseWidget();
    private slots:
        void  onBlockSizeChange(int);
        void  onEntireCheckBoxChange(bool);
    private:
        Ui::ReverseWidget *ui;
        Reverse *transform;
};

#endif // REVERSEWIDGET_H
