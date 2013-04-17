/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HIEROGLYPHYWIDGET_H
#define HIEROGLYPHYWIDGET_H

#include <QWidget>
#include "../hieroglyphy.h"

namespace Ui {
class HieroglyphyWidget;
}

class HieroglyphyWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HieroglyphyWidget(Hieroglyphy * transform, QWidget *parent = 0);
        ~HieroglyphyWidget();
        
    private slots:
        void on_btoaCheckBox_clicked();

    private:
        Ui::HieroglyphyWidget *ui;
        Hieroglyphy *transform;
};

#endif // HIEROGLYPHYWIDGET_H
