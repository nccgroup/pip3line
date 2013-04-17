/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PADDINGWIDGET_H
#define PADDINGWIDGET_H

#include <QWidget>
#include "../padding.h"

namespace Ui {
class PaddingWidget;
}

class PaddingWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit PaddingWidget(Padding *ntransform, QWidget *parent = 0);
        ~PaddingWidget();
    private slots:
        void onTypeChange(int index);
        void onPaddingCharChange(QString);
        void onBlockSizeChange(int size);
    private:
        Ui::PaddingWidget *ui;
        Padding *transform;
};

#endif // PADDINGWIDGET_H
