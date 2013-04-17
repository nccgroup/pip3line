/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/
#ifndef URLENCODEWIDGET_H
#define URLENCODEWIDGET_H

#include <QWidget>
#include "../urlencode.h"

namespace Ui {
class UrlEncodeWidget;
}

class UrlEncodeWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit UrlEncodeWidget(UrlEncode *ntransform, QWidget *parent = 0);
        ~UrlEncodeWidget();
    private slots:
        void onIncludeAll();
        void onExcludeChange(QString vals);
        void onIncludeChange(QString vals);
        void onPercentChange(QString character);
    private:
        Ui::UrlEncodeWidget *ui;
        UrlEncode *transform;
};

#endif // URLENCODEWIDGET_H
