/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESTOFLOATWDIGET_H
#define BYTESTOFLOATWDIGET_H

#include <QWidget>
#include "../bytestofloat.h"

namespace Ui {
class BytesToFloatWdiget;
}

class BytesToFloatWdiget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit BytesToFloatWdiget(BytesToFloat *transform,QWidget *parent = 0);
        ~BytesToFloatWdiget();
        
    private slots:
        void onLittleEndianChange(bool checked);
        void on32BitSizeChanged(bool checked);
        void onPrecisionChanged(int val);

    private:
        Ui::BytesToFloatWdiget *ui;
        BytesToFloat *transform;
};

#endif // BYTESTOFLOATWDIGET_H
