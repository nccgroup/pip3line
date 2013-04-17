/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BASEXWIDGET_H
#define BASEXWIDGET_H

#include <QWidget>
#include "../basex.h"

namespace Ui {
class BaseXWidget;
}

class BaseXWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit BaseXWidget(BaseX *transform, QWidget *parent = 0);
        ~BaseXWidget();
    private slots:
        void onBaseChange(int val);
        void onUpperCaseChange(bool val);
    private:
        Ui::BaseXWidget *ui;
        BaseX *transform;
};

#endif // BASEXWIDGET_H
