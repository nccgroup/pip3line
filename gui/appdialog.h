/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef APPDIALOG_H
#define APPDIALOG_H

#include <QDialog>
#include <QPoint>
class GuiHelper;
class LoggerWidget;
class QAction;

class AppDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit AppDialog(GuiHelper * guiHelper, QWidget *parent = 0);
        void attachAction(QAction * action);
    signals:
        void hiding();
    protected slots:
        void onMainHiding();
        void onMainRising();
    protected:
        void showEvent(QShowEvent * event);
        GuiHelper * guiHelper;
        bool savedVisibleState;
        QPoint savedPos;
        LoggerWidget *logger;
        void hideEvent(QHideEvent *event);
        QAction * attachedAction;
};

#endif // APPDIALOG_H
