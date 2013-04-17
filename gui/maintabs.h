/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MAINTABS_H
#define MAINTABS_H

#include <QTabWidget>
#include <QDialog>
#include <QTabBar>
#include <QHash>
#include <QNetworkAccessManager>
#include <transformmgmt.h>
#include "guihelper.h"
#include "transformsgui.h"
#include "ui_tabname.h"
#include "loggerwidget.h"
#include "floatingdialog.h"

class MainTabs : public QTabWidget
{
        Q_OBJECT
    public:
        explicit MainTabs(GuiHelper *guiHelper, QWidget *parent = 0);
        ~MainTabs();
        bool eventFilter(QObject * receiver, QEvent * event);
        void askForRenaming(int index);
    public slots:
        void newTabTransform(const QByteArray &initialValue = QByteArray(), const QString &conf = QString());
        void onDeleteTab(int index);
        void showLogs();
    private slots:
        void onLogError();
        void onLogCleanStatus();
        void receivedNameChanged(TransformsGui * tab);
        void receivedTabWindowSwitch(TransformsGui * tab);
        void receivedBringToFront(TransformsGui * tab);
        void onFloatingWindowsReject();
    private:
        static const QString ID;
        QTabBar * tabBarRef;
        GuiHelper *guiHelper;
        LoggerWidget * logger;
        QHash<TransformsGui *,bool> tabList;
        QHash<TransformsGui *, FloatingDialog *> activeWindows;
        uint tabCount;
};

#endif // MAINTABS_H
