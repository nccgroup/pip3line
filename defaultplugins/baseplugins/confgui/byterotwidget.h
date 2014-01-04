/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTEROTWIDGET_H
#define BYTEROTWIDGET_H

#include <QWidget>

namespace Ui {
class ByteRotWidget;
}

class ByteRot;

class ByteRotWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ByteRotWidget(ByteRot * transf, QWidget *parent = 0);
        ~ByteRotWidget();
    private slots:
        void onRotationChanged(int val);
    private:
        Ui::ByteRotWidget *ui;
        ByteRot *transform;
};

#endif // BYTEROTWIDGET_H
