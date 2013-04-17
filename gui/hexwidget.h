/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXWIDGET_H
#define HEXWIDGET_H

#include <QWidget>

namespace Ui {
class HexWidget;
}

class HexWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit HexWidget(QWidget *parent = 0);
        ~HexWidget();
        char getChar();
        void setChar(char c);
    signals:
        void charChanged(char c);
    private slots:
        void onHexChanged();
    private:
        Ui::HexWidget *ui;
};

#endif // HEXWIDGET_H
