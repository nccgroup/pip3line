/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "maintabs.h"
#include "textinputdialog.h"
#include "tabs/transformsgui.h"
#include "tabs/generictab.h"
#include <QMouseEvent>
#include <QFileInfo>
#include <QDebug>

const QString MainTabs::ID = "MainTabs";
const uint MainTabs::DEFAULT_MAX_TAB_COUNT = 50;

MainTabs::MainTabs(GuiHelper *nguiHelper, QWidget *parent) :
    QTabWidget(parent)
{
    tabBarRef = tabBar();
    tabBarRef->installEventFilter(this);

    tabCount = 1;
    maxTabCount = DEFAULT_MAX_TAB_COUNT;

    setTabsClosable(true);
    setMovable(true);

    guiHelper = nguiHelper;
    connect(guiHelper, SIGNAL(newTabRequested(QByteArray)), this, SLOT(newTabTransform(QByteArray)));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onDeleteTab(int)));

    logger = guiHelper->getLogger();
    if (logger != NULL) {
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
    QHashIterator<TabAbstract *, bool> i(tabList);
    while (i.hasNext()) {
     i.next();
     delete i.key();
    }
    tabList.clear();
    logger = NULL;
    guiHelper = NULL;
    tabBarRef = NULL;
}

bool MainTabs::eventFilter(QObject *receiver, QEvent *event)
{
    bool result = QObject::eventFilter(receiver, event);
    if (receiver == tabBarRef) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me == NULL) {
                qWarning() << "[MainTabs::eventFilter] NULL MouseEvent";
            } else {
                // checking if we can locate the tab

                int clickedTabId = tabBarRef->tabAt(me->pos());
                if (clickedTabId == -1)
                    return result;
                //renaming
                askForRenaming(clickedTabId);
                return true;  //no further handling of this event is required
            }
        }
        // not usefull right now, probably never will...
//            else if (event->type() == QEvent::MouseButtonPress) {
//                QPoint posGlobal = me->globalPos();
//                int clickedTabId = tabBarRef->tabAt(me->pos());
//                qDebug() << "pressing on tab " << clickedTabId << " x:" << posGlobal.rx() << " y:" << posGlobal.ry();
//            } else if (event->type() == QEvent::MouseButtonRelease) {
//                QPoint posGlobal = me->globalPos();
//                qDebug() << "releasing on tab" << posGlobal.rx() << " y:" << posGlobal.ry();
//            }  else if (event->type() == QEvent::MouseMove) {

//                QPoint posGlobal = me->globalPos();
//                qDebug() << "Moving on tab" << posGlobal.rx() << posGlobal.ry();
//            }
        }
    return result;
}

void MainTabs::askForRenaming(int index)
{
    if (index != indexOf(logger)) {
        TextInputDialog * dia = guiHelper->getNameDialog(this, tabBarRef->tabText(index));
        if (dia != NULL) {
            int res = dia->exec();
            QString newName = dia->getInputText();
            if (res == QDialog::Accepted && !newName.isEmpty()) {
                static_cast<TabAbstract *>(widget(index))->setName(newName);
            }

            delete dia;
        }
    }
}

void MainTabs::loadFile(QString fileName)
{
    int index = 0;
    if (count() - (indexOf(logger) == -1? 0 : 1) == 0) {
        index = newTabTransform();
    }

    if (index > -1) {
        TabAbstract * tg = static_cast<TabAbstract *>(widget(index));
        tg->loadFromFile(fileName);
    }
}

int MainTabs::integrateTab(TabAbstract *newTab)
{
    int nextInsert = -1;
    if (tabCount > maxTabCount) {
        logger->logError(tr("Reached the maximum number of allowed tabs"));
        return nextInsert;
    }

    nextInsert = count() - ((indexOf(logger) == -1)? 0 : 1);
    connect(newTab, SIGNAL(askWindowTabSwitch()), this, SLOT(receivedTabWindowSwitch()));

    tabList.insert(newTab,false);

    QString tabName = newTab->getName();
    if (tabName.isEmpty()) {
        tabName = tr("%1").arg(tabCount);
        newTab->setName(tabName);
    }

    connect(newTab, SIGNAL(nameChanged()), this, SLOT(receivedNameChanged()));
    connect(newTab, SIGNAL(askBringFront()), this, SLOT(receivedBringToFront()));
    insertTab(nextInsert, newTab, tabName);
    setCurrentIndex(nextInsert);
    guiHelper->addTab(newTab);
    tabCount++;
    return nextInsert;
}

int MainTabs::newTabTransform(const QByteArray &initialValue, const QString &conf)
{
    int ret = -1;
    if (tabCount > maxTabCount) {
        logger->logError(tr("Reached the maximum number of allowed tabs"));
        return ret;
    }

    TransformsGui *newTab = new(std::nothrow) TransformsGui(guiHelper, this);
    if (newTab != NULL) {
        ret = integrateTab(newTab);
        newTab->setCurrentChainConf(conf);
        if (!initialValue.isEmpty())
            newTab->setData(initialValue);
        guiHelper->raisePip3lineWindow();
    } else {
        qFatal("Cannot allocate memory for newTab X{");
    }

    return ret;
}

void MainTabs::onDeleteTab(int index)
{
    if (index != indexOf(logger)) {
        TabAbstract *tgui = static_cast<TabAbstract *>(widget(index));
        // index is set by Qt, no need to verify

        removeTab(index);
        tabList.remove(tgui);
        delete tgui;
        setCurrentIndex(index > 0 ? index - 1 : 0);
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
    if (index == -1 && logger != NULL) {
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

void MainTabs::receivedNameChanged()
{
    TabAbstract *tab = dynamic_cast<TabAbstract *>(sender());
    int index = indexOf(tab);
    if (index == -1) {
        qWarning("[MainTabs] Tab %d not found when renaming T_T",index);
        return;
    }
    QString name = tab->getName();
    if (index != -1 && !name.isEmpty()) {
        tabBarRef->setTabText(index,name);
        guiHelper->addTab(tab);
    }
}

void MainTabs::receivedTabWindowSwitch()
{
    TabAbstract *tab = static_cast<TabAbstract *>(sender());
    if (!tabList.contains(tab)) {
        qWarning("[MainTabs] Tab not found when switching window <-> tab T_T");
        return;
    } else {
        if (tabList.value(tab)) { // window -> tab
            addTab(tab, tab->getName());
            setCurrentWidget(tab);
            if (!activeWindows.contains(tab)) {
                qWarning("[MainTabs] Widget not found in the active windows (window -> tab) T_T");
            } else {
               FloatingDialog *fd = activeWindows.value(tab);
               activeWindows.remove(tab);
               delete fd;
            }
            tabList.insert(tab,false);

        } else { // tab -> window
            detachTab(tab);
        }
    }
}

void MainTabs::detachTab(TabAbstract *tab)
{
    int index = indexOf(tab);
    if (index == -1) {
        qWarning("[MainTabs] Tab not found when switching tab -> window T_T");
    } else {

        removeTab(index);
        if (count() != 0)
            setCurrentIndex(0);
        FloatingDialog *fd = new(std::nothrow) FloatingDialog(guiHelper, tab, this);
        if (fd != NULL) {
            activeWindows.insert(tab, fd);
            fd->setWindowTitle(tab->getName());
            fd->raise();
            fd->show();
            tabList.insert(tab,true);
            connect(fd,SIGNAL(rejected()), this, SLOT(onFloatingWindowsReject()));
        } else {
            qFatal("Cannot allocate memory for FloatingDialog X{");
        }
    }
}

void MainTabs::receivedBringToFront()
{
    TabAbstract *tab = static_cast<TabAbstract *>(sender());
    if (!tabList.contains(tab)) {
        qWarning("[MainTabs] Tab not found when bringing to front T_T");
        return;
    } else {
        if (tabList.value(tab)) { // windows
            if (!activeWindows.contains(tab)) {
                qWarning("[MainTabs] Widget not found in the active windows (bringToFront) T_T");
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
    FloatingDialog *fd = dynamic_cast<FloatingDialog *>(sender());
    if (fd != NULL) {
        TabAbstract *tab = activeWindows.key(fd,NULL);
        if (tab != NULL) {
            addTab(tab, tab->getName());
            setCurrentWidget(tab);
            activeWindows.remove(tab);
            delete fd;
            tabList.insert(tab,false);
        } else {
           qWarning("[MainTabs] Tab not found  in the active windows tab (reject) T_T");
        }
    }
}
