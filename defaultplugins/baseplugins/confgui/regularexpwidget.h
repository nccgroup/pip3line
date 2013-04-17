/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef REGULAREXPWIDGET_H
#define REGULAREXPWIDGET_H

#include <QWidget>
#include "../regularexp.h"

namespace Ui {
class RegularExpWidget;
}

class RegularExpWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit RegularExpWidget(RegularExp *ntransform, QWidget *parent = 0);
        ~RegularExpWidget();
    private slots:
        void onActionTypeChange(int val);
        void onGreedyChange(bool val);
        void onRegularExpressionChange(QString val);
        void onCaseInsensitiveChange(bool val);
        void onGroupChange(int val);
        void onAllGroupsChange(bool val);
        void onReplacementStringChange(QString val);
        void onLinByLineChange(bool val);
    private:
        Ui::RegularExpWidget *ui;
        RegularExp *transform;
};

#endif // REGULAREXPWIDGET_H
