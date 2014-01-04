/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HMACTRANSFORMWIDGET_H
#define HMACTRANSFORMWIDGET_H

#include <QWidget>

namespace Ui {
class HMACTransformWidget;
}

class HMACTransform;

class HMACTransformWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit HMACTransformWidget(HMACTransform *transform, QWidget *parent = 0);
        ~HMACTransformWidget();

    private slots:
        void onHashSelected(int index);
        void onKeyLineReturn();

    private:
        Ui::HMACTransformWidget *ui;
        HMACTransform *transform;
};

#endif // HMACTRANSFORMWIDGET_H
