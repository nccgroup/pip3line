/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef XORWIDGET_H
#define XORWIDGET_H

#include <QWidget>
#include "../xor.h"

namespace Ui {
class XorWidget;
}

class XorWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit XorWidget(Xor *ntransform, QWidget *parent = 0);
        ~XorWidget();

    private slots:
        void onFromHexChange(bool val);
        void onKeyChange();
        void onTypeChange(int index);
    private:
        Ui::XorWidget *ui;
        Xor *transform;
        bool eventFilter(QObject *o, QEvent *e);
        void focusOutEvent ( QFocusEvent * event );
};

#endif // XORWIDGET_H
