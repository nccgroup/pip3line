/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef IPTRANSLATEWIDGET_H
#define IPTRANSLATEWIDGET_H

#include <QWidget>
#include "../iptranslateipv4.h"

namespace Ui {
class IPTranslateIPv4Widget;
}

class IPTranslateIPv4Widget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit IPTranslateIPv4Widget(IPTranslateIPv4 *transform, QWidget *parent = 0);
        ~IPTranslateIPv4Widget();

    private slots:
        void endianChanged();
        void caseChanged();
        void baseChanged();

    private:
        Ui::IPTranslateIPv4Widget *ui;
        IPTranslateIPv4 *transform;
};

#endif // IPTRANSLATEWIDGET_H
