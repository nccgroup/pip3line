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
#include "sources/bytesourceabstract.h"
#include "guihelper.h"
#include "tabs/tababstract.h"
#include "loggerwidget.h"
#include "floatingdialog.h"
#include <QDragMoveEvent>

class MainTabs : public QTabWidget
{
        Q_OBJECT
    public:
        explicit MainTabs(GuiHelper *guiHelper, QWidget *parent = 0);
        ~MainTabs();

        void askForRenaming(int index);
        void loadFile(QString fileName);
        int integrateTab(TabAbstract * newTab);
        static const uint DEFAULT_MAX_TAB_COUNT;
    public slots:
        int newTabTransform(const QByteArray &initialValue = QByteArray(), const QString &conf = QString());
        void onDeleteTab(int index);
        void showLogs();
    private slots:
        void onLogError();
        void onLogCleanStatus();
        void receivedNameChanged();
        void receivedTabWindowSwitch();
        void detachTab(TabAbstract * tab);
        void receivedBringToFront();
        void onFloatingWindowsReject();
    private:
        Q_DISABLE_COPY(MainTabs)
        static const QString ID;
        bool eventFilter(QObject * receiver, QEvent * event);

        QTabBar * tabBarRef;
        GuiHelper *guiHelper;
        LoggerWidget * logger;
        QHash<TabAbstract *,bool> tabList;
        QHash<TabAbstract *, FloatingDialog *> activeWindows;
        uint tabCount;
        uint maxTabCount;
};

#endif // MAINTABS_H
