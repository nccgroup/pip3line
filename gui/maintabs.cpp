/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "maintabs.h"
#include <QMouseEvent>

const QString MainTabs::ID = "MainTabs";

MainTabs::MainTabs(GuiHelper *nguiHelper, QWidget *parent) :
    QTabWidget(parent)
{
    tabBarRef = tabBar();
    tabBarRef->installEventFilter(this);

    tabCount = 1;

    setTabsClosable(true);
    setMovable(true);

    guiHelper = nguiHelper;
    connect(guiHelper, SIGNAL(newTabRequested(QByteArray)), this, SLOT(newTabTransform(QByteArray)));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onDeleteTab(int)));

    logger = guiHelper->getLogger();
    if (logger != 0) {
        connect(logger, SIGNAL(errorRaised()), this, SLOT(onLogError()));
        connect(logger,SIGNAL(cleanStatus()), this, SLOT(onLogCleanStatus()));
        int index = this->addTab(logger,tr("Logs"));
        if (logger->hasUncheckedError())
            tabBarRef->setTabTextColor(index, Qt::red);
    }
    newTabTransform();
}

MainTabs::~MainTabs()
{
    QHashIterator<TransformsGui *, bool> i(tabList);
    while (i.hasNext()) {
     i.next();
     delete i.key();
    }
    tabList.clear();
}

bool MainTabs::eventFilter(QObject *receiver, QEvent *event)
{
    bool result = QObject::eventFilter(receiver, event);
    if (receiver == tabBarRef) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                // checking if we can locate the tab
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                int clickedTabId = tabBarRef->tabAt(me->pos());
                if (clickedTabId == -1)
                    return result;
                //renaming
                askForRenaming(clickedTabId);
                return true; //no further handling of this event is required
            }
        }
    return result;
}

void MainTabs::askForRenaming(int index)
{
    if (index != indexOf(logger)) {
        NameDialog * dia = guiHelper->getNameDialog(this, tabBarRef->tabText(index));

        int res = dia->exec();
        if (res == QDialog::Accepted && !dia->getName().isEmpty()) {
            ((TransformsGui *)widget(index))->setName(dia->getName());
        }

        delete dia;
    }
}

void MainTabs::newTabTransform(const QByteArray &initialValue, const QString &conf)
{
    int nextInsert = count() - ((indexOf(logger) == -1)? 0 : 1);
    TransformsGui *newTab = new TransformsGui(guiHelper, this);
    connect(newTab, SIGNAL(askWindowTabSwitch(TransformsGui*)), this, SLOT(receivedTabWindowSwitch(TransformsGui*)));
    newTab->setCurrentChainConf(conf);
    tabList.insert(newTab,false);
    newTab->setData(initialValue);
    QString tabName = newTab->getName();
    if (tabName.isEmpty()) {
        tabName = tr("%1").arg(tabCount);
        newTab->setName(tabName);
    }

    connect(newTab, SIGNAL(nameChanged(TransformsGui*)), this, SLOT(receivedNameChanged(TransformsGui*)));
    connect(newTab, SIGNAL(askBringFront(TransformsGui*)), this, SLOT(receivedBringToFront(TransformsGui*)));
    insertTab(nextInsert, newTab, tabName);
    setCurrentIndex(nextInsert);
    guiHelper->addTab(newTab);
    tabCount++;
}

void MainTabs::onDeleteTab(int index)
{
    if (index != indexOf(logger)) {
        TransformsGui *tgui = (TransformsGui *)widget(index);
        // index is set by Qt, no need to verify

        removeTab(index);
        tabList.remove(tgui);
        delete tgui;
    } else {
        removeTab(index);
    }
}

void MainTabs::showLogs()
{
    int index = indexOf(logger);
    if (index == -1) {
        index = addTab(logger, tr("Logs"));
    }
    setCurrentIndex(index);
}

void MainTabs::onLogError()
{
    int index = indexOf(logger);
    if (index == -1 && logger != 0) {
        index = addTab(logger, tr("Logs"));
    }
    tabBarRef->setTabTextColor(index, Qt::red);
}

void MainTabs::onLogCleanStatus()
{
    int index = indexOf(logger);
    if (index != -1) {
        tabBarRef->setTabTextColor(index, Qt::black);
    }
}

void MainTabs::receivedNameChanged(TransformsGui *tab)
{
    int index = indexOf(tab);
    if (index == -1) {
        logger->logError(tr("Tab %1 not found when renaming T_T").arg(index),ID);
    }
    QString name = tab->getName();
    if (index != -1 && !name.isEmpty()) {
        tabBarRef->setTabText(index,name);
        guiHelper->addTab(tab);
    }
}

void MainTabs::receivedTabWindowSwitch(TransformsGui *tab)
{
    if (!tabList.contains(tab)) {
        logger->logError(tr("Tab %1 not found when switching window <-> tab T_T").arg(tab->getName()),ID);
        return;
    } else {
        if (tabList.value(tab)) { // window
            addTab(tab, tab->getName());
            setCurrentWidget(tab);
            if (!activeWindows.contains(tab)) {
                logger->logError(tr("Widget %1 not found in the active windows T_T").arg(tab->getName()),ID);
            } else {
               FloatingDialog *fd = activeWindows.value(tab);
               activeWindows.remove(tab);
               delete fd;
            }
            tabList.insert(tab,false);

        } else { // tab
            int index = indexOf(tab);
            if (index == -1) {
                logger->logError(tr("Tab %1 not found when switching window <-> tab T_T").arg(index),ID);
            } else {
                removeTab(index);
                if (count() != 0)
                    setCurrentIndex(0);
                FloatingDialog *fd = new FloatingDialog(tab, this);
                activeWindows.insert(tab, fd);
                fd->setWindowTitle(tab->getName());
                fd->raise();
                fd->show();
                tabList.insert(tab,true);
                connect(fd,SIGNAL(rejected()), this, SLOT(onFloatingWindowsReject()));
            }
        }
    }
}

void MainTabs::receivedBringToFront(TransformsGui *tab)
{
    if (!tabList.contains(tab)) {
        logger->logError(tr("Tab %1 not found when bringing to front T_T").arg(tab->getName()),ID);
        return;
    } else {
        if (tabList.value(tab)) { // windows
            if (!activeWindows.contains(tab)) {
                logger->logError(tr("Widget %1 not found in the active windows (bringToFront) T_T").arg(tab->getName()),ID);
            } else {
                FloatingDialog *fd = activeWindows.value(tab);
                fd->raise();
                fd->show();
            }
        } else { // tab
            setCurrentWidget(tab);
        }
    }
}

void MainTabs::onFloatingWindowsReject()
{
    FloatingDialog *fd = (FloatingDialog *) sender();
    if (fd != 0) {
        TransformsGui *tab = activeWindows.key(fd,0);
        if (tab != 0) {
            addTab(tab, tab->getName());
            setCurrentWidget(tab);
            activeWindows.remove(tab);
            delete fd;
            tabList.insert(tab,false);
        } else {
           logger->logError(tr("Tab not found  in the active windows tab (reject) T_T"),ID);
        }
    }
}
