/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DISTORMWIDGET_H
#define DISTORMWIDGET_H

#include <QWidget>
#include <QStringList>
#include "distormtransf.h"

namespace Ui {
class DistormWidget;
}

class DistormWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit DistormWidget(DistormTransf * transform, QWidget *parent = 0);
        ~DistormWidget();
    private slots:
        void onMaxInstruc(int val);
        void onOffset(QString val);
        void onType(int val);
    private:
        static const QStringList asmTypeStrings;
        Ui::DistormWidget *ui;
        DistormTransf * transform;
};

#endif // DISTORMWIDGET_H
