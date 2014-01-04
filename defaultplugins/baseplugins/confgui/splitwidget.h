/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SPLITWIDGET_H
#define SPLITWIDGET_H

#include <QWidget>
#include "../split.h"

namespace Ui {
class SplitWidget;
}

class SplitWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit SplitWidget(Split *ntransform, QWidget *parent = 0);
        ~SplitWidget();
    private slots:
        void onSeparatorChange(QString);
        void onGroupChange(int);
        void onAllGroupChange(bool);
        void onTrimChange(bool val);
        void onProcLineChanged(bool val);
    private:
        Ui::SplitWidget *ui;
        Split *transform;
};

#endif // SPLITWIDGET_H
