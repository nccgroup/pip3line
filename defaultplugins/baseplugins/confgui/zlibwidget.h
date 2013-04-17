/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ZLIBWIDGET_H
#define ZLIBWIDGET_H

#include <QWidget>
#include "../zlib.h"

namespace Ui {
class ZlibWidget;
}

class ZlibWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ZlibWidget(Zlib * transform, QWidget *parent = 0);
        ~ZlibWidget();
        
    private slots:
        void on_compressionSpinBox_valueChanged(int value);

        void on_removeHeaderCheckBox_toggled(bool checked);

    private:
        Ui::ZlibWidget *ui;
        Zlib * transform;
};

#endif // ZLIBWIDGET_H
