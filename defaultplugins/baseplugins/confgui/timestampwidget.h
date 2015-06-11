/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TIMESTAMPWIDGET_H
#define TIMESTAMPWIDGET_H

#include <QWidget>
#include "../timestamp.h"

namespace Ui {
class TimestampWidget;
}

class TimestampWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit TimestampWidget(TimeStamp *transform, QWidget *parent = 0);
        ~TimestampWidget();

    private slots:
        void formatChanged(QString format);
        void outBoundTZLocalChanged(bool checked);
        
    private:
        Ui::TimestampWidget *ui;
        TimeStamp *transform;
};

#endif // TIMESTAMPWIDGET_H
