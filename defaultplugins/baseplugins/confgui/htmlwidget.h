/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HTMLWIDGET_H
#define HTMLWIDGET_H

#include <QWidget>
#include "../html.h"

namespace Ui {
class HtmlWidget;
}

class HtmlWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HtmlWidget(Html *ntransform, QWidget *parent = 0);
        ~HtmlWidget();
    private slots:
            void onChangeScope();
            void onChangeType();
            void onChangeUseName();
        
    private:
        Ui::HtmlWidget *ui;
        Html *transform;
};

#endif // HTMLWIDGET_H
