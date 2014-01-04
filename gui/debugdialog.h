/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include "appdialog.h"
#include <QModelIndex>

namespace Ui {
class DebugDialog;
}

class GuiHelper;
class HexView;
class CurrentMemorysource;
class QEvent;


class DebugDialog : public AppDialog
{
        Q_OBJECT
    public:
        explicit DebugDialog(GuiHelper* helper, QWidget *parent = 0);
        ~DebugDialog();
        
        void closeEvent(QCloseEvent *event);
    private slots:
        void onLoad();
        void refreshTransformInstances();
        void onInstanceClick(QModelIndex index);
    private:
        Q_DISABLE_COPY(DebugDialog)
        void loadQObject(QObject * obj);
        bool eventFilter(QObject *o, QEvent *event);
        Ui::DebugDialog *ui;
        HexView *hexview;
        CurrentMemorysource *source;
};

#endif // DEBUGDIALOG_H
