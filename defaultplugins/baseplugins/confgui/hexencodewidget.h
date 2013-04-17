/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXENCODEWIDGET_H
#define HEXENCODEWIDGET_H

#include <QWidget>
#include "../hexencode.h"

namespace Ui {
class HexEncodeWidget;
}

class HexEncodeWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HexEncodeWidget(HexEncode *ntransform, QWidget *parent = 0);
        ~HexEncodeWidget();
    private slots:
        void onTypeChange();
    private:
        Ui::HexEncodeWidget *ui;
        HexEncode *transform;
};

#endif // HEXENCODEWIDGET_H
