/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MICROSOFTTIMESTAMPWIDGET_H
#define MICROSOFTTIMESTAMPWIDGET_H

#include <QWidget>
#include "../microsofttimestamp.h"

namespace Ui {
class MicrosoftTimestampWidget;
}

class MicrosoftTimestampWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit MicrosoftTimestampWidget(MicrosoftTimestamp * transform, QWidget *parent = 0);
        ~MicrosoftTimestampWidget();

    private slots:
        void formatChanged(QString format);
        void onNSChanged(QString nsString);
        void localTzToggled(bool checked);
        
    private:
        Ui::MicrosoftTimestampWidget *ui;
        MicrosoftTimestamp * transform;
};

#endif // MICROSOFTTIMESTAMPWIDGET_H
