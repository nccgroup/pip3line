/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "generictab.h"
#include "ui_generictab.h"
#include <QDebug>
#include "../shared/offsetgotowidget.h"
#include "../shared/searchwidget.h"
#include "../sources/bytesourceabstract.h"
#include "../views/hexview.h"
#include "../views/textview.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QKeyEvent>
#include <QAction>
#include <QTabWidget>
#include <QDropEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include "../quickviewitemconfig.h"
#include <QUrl>
#include "../downloadmanager.h"
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../shared/readonlybutton.h"
#include "../shared/clearallmarkingsbutton.h"
#include "../shared/bytesourceguibutton.h"
#include "../shared/detachtabbutton.h"
#include "../shared/messagepanelwidget.h"
#include "../sources/intermediatesource.h"
#include "../shared/universalreceiverbutton.h"

GenericTab::GenericTab(ByteSourceAbstract *bytesource, GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent)
{
    ableToReceiveData = false;
    byteSource = bytesource;
    setName(bytesource->name());
    connect(byteSource,SIGNAL(nameChanged(QString)), SLOT(setName(QString)));

    hexView = new(std::nothrow) HexView(byteSource, guiHelper,this);
    if (hexView == NULL) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui = new(std::nothrow) Ui::GenericTab();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::FileTab X{");
    }
    ui->setupUi(this);
    ui->tabWidget->addTab(hexView, tr("Hex"));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onDeleteTab(int)));

    searchWidget = new(std::nothrow) SearchWidget(byteSource, this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    ui->mainLayout->insertWidget(1,searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,QBitArray,bool)), SLOT(onSearch(QByteArray,QBitArray,bool)));

    hexView->installEventFilter(this);

    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper,this);
    if (gotoWidget == NULL) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }
    gotoWidget->setMaximumWidth(150);
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1, gotoWidget);
    connect(gotoWidget,SIGNAL(gotoRequest(quint64,bool,bool,bool)), SLOT(onGotoOffset(quint64,bool,bool,bool)));

    roButton = new(std::nothrow) ReadOnlyButton(byteSource,this);
    if (roButton == NULL) {
        qFatal("Cannot allocate memory for ReadOnlyButton X{");
    }
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 2,roButton);

    clearAllMarksButton = new(std::nothrow) ClearAllMarkingsButton(byteSource,this);
    if (clearAllMarksButton == NULL) {
        qFatal("Cannot allocate memory for clearAllMarkingsButton X{");
    }
    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(roButton) + 1, clearAllMarksButton);

    guiButton = new(std::nothrow) ByteSourceGuiButton(byteSource,guiHelper,this);
    if (guiButton == NULL) {
        qFatal("Cannot allocate memory for ByteSourceGuiButton X{");
    }

    ui->mainToolBarLayout->insertWidget(5, guiButton);

    integrateByteSource();

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == NULL) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(ui->loadPushButton) + 1,detachButton);

    messagePanel = new(std::nothrow) MessagePanelWidget(this);
    if (messagePanel == NULL) {
        qFatal("Cannot allocate memory for MessagePanelWidget X{");
    }


    ui->mainLayout->insertWidget(ui->mainLayout->indexOf(ui->tabWidget) + 1,messagePanel);
    connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), messagePanel, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));
    //connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));

    // Checking if there are some additional buttons
    QWidget *gui = byteSource->getGui(this, ByteSourceAbstract::GUI_BUTTONS);
    if (gui != NULL) {
        ui->mainToolBarLayout->insertWidget(4, gui);
    }

    // Checking if there is an additional upper pane
    gui = byteSource->getGui(this, ByteSourceAbstract::GUI_UPPER_VIEW);
    if (gui != NULL) {
        ui->mainLayout->insertWidget(1,gui);
    }

    QMenu * newViewMenu = new(std::nothrow)QMenu(ui->addViewPushButton);
    if (newViewMenu == NULL) {
        qFatal("Cannot allocate memory for QMenu X{");
    }

    QAction * descAction = new(std::nothrow)QAction(tr("View type"),newViewMenu);
    if (descAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    descAction->setDisabled(true);
    newViewMenu->addAction(descAction);


    newHexViewAction = new(std::nothrow)QAction(tr("Hexadecimal"),newViewMenu);
    if (newHexViewAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    newViewMenu->addAction(newHexViewAction);

    newTextViewAction = new(std::nothrow)QAction(tr("Text"),newViewMenu);
    if (newTextViewAction == NULL) {
        qFatal("Cannot allocate memory for QAction X{");
    }
    newViewMenu->addAction(newTextViewAction);
    ui->addViewPushButton->setMenu(newViewMenu);

    UniversalReceiverButton *urb = new(std::nothrow) UniversalReceiverButton(this, guiHelper);
    if (urb == NULL) {
        qFatal("Cannot allocate memory for UniversalReceiverButton X{");
    }

    ui->mainToolBarLayout->insertWidget(ui->mainToolBarLayout->indexOf(gotoWidget) + 1,urb);

    connect(newViewMenu,SIGNAL(triggered(QAction*)), SLOT(onNewViewTab(QAction*)));

    connect(hexView,SIGNAL(askForFileLoad()), SLOT(fileLoadRequest()));
    connect(ui->loadPushButton, SIGNAL(clicked()), SLOT(fileLoadRequest()));

    connect(ui->historyUndoPushButton, SIGNAL(clicked()), SLOT(onHistoryBackward()));
    connect(ui->historyRedoPushButton, SIGNAL(clicked()), SLOT(onHistoryForward()));

    setAcceptDrops(true);
}

GenericTab::~GenericTab()
{
    delete gotoWidget;
    delete searchWidget;
    delete detachButton;
    delete guiButton;
    delete byteSource;
    delete ui;
}

void GenericTab::loadFromFile(QString fileName)
{
    if (fileName.isEmpty()) {
        logger->logError("Empty file name, ignoring",metaObject()->className());
        return;
    }

    if (byteSource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        byteSource->fromLocalFile(fileName);
        integrateByteSource();
    }  else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)byteSource)->metaObject()->className()),QMessageBox::Ok);
    }
}

int GenericTab::getBlockCount() const
{
    return 1;
}

ByteSourceAbstract *GenericTab::getSource(int)
{
    return byteSource;
}

ByteTableView *GenericTab::getHexTableView(int)
{
    return hexView->getHexTableView();
}

void GenericTab::setData(const QByteArray &data)
{
    if (byteSource->isReadonly()) {
        QMessageBox::critical(this, tr("Read only"), tr("The byte source is readonly"),QMessageBox::Ok);
    } else if (byteSource->hasCapability(ByteSourceAbstract::CAP_RESET)) {
        byteSource->setData(data);
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_RESET capability").arg(((QObject *)byteSource)->metaObject()->className()),QMessageBox::Ok);
    }
}

bool GenericTab::canReceiveData()
{
    return (byteSource != NULL && byteSource->hasCapability(ByteSourceAbstract::CAP_RESET));
}

void GenericTab::fileLoadRequest()
{
    QString fileName;
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            byteSource->fromLocalFile(fileName);
            integrateByteSource();
            setName(QFileInfo(fileName).fileName());
        }
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_LOADFILE capability, ignoring").arg(((QObject *)byteSource)->metaObject()->className()),QMessageBox::Ok);
    }
}

void GenericTab::onGotoOffset(quint64 offset, bool absolute, bool negative, bool select)
{
    if (!hexView->goTo(offset,absolute,negative, select)) {
        gotoWidget->setStyleSheet(GuiStyles::LineEditError);
    } else {
        gotoWidget->setStyleSheet(qApp->styleSheet());
    }
}

void GenericTab::onHistoryBackward()
{
    byteSource->historyBackward();
}

void GenericTab::onHistoryForward()
{
    byteSource->historyForward();
}

void GenericTab::onNewViewTab(QAction *action)
{
    QuickViewItemConfig *itemConfig = new(std::nothrow) QuickViewItemConfig(guiHelper, this);
    if (itemConfig == NULL) {
        qFatal("Cannot allocate memory for QuickViewItemConfig X{");
    }
    itemConfig->setWayBoxVisible(true);
    itemConfig->setFormatVisible(false);
    int ret = itemConfig->exec();
    if (ret == QDialog::Accepted) {
        TransformAbstract * ta = itemConfig->getTransform();
        QString newName = itemConfig->getName();
        IntermediateSource * is = new(std::nothrow) IntermediateSource(guiHelper,byteSource,ta);
        if (is == NULL) {
            qFatal("Cannot allocate memory for IntermediateSource X{");
        }
        QTabWidget * thetabwiget = ui->tabWidget;
        SingleViewAbstract * newView = NULL;

        if (action == newHexViewAction) {
            newView = new(std::nothrow) HexView(is,guiHelper,thetabwiget);
            if (newView == NULL) {
                qFatal("Cannot allocate memory for HexView X{");
            }
        } else {
            newView = new(std::nothrow) TextView(is,guiHelper,thetabwiget);
            if (newView == NULL) {
                qFatal("Cannot allocate memory for TextView X{");
            }
        }

        thetabwiget->addTab(newView,newName);
    }
    delete itemConfig;
}

void GenericTab::onDeleteTab(int index)
{
    if (ui->tabWidget->indexOf(hexView) == index) {
        logger->logWarning(tr("Cannot close the main hexadecimal tab"),metaObject()->className());
    } else {
        SingleViewAbstract * sva = static_cast<SingleViewAbstract *>(ui->tabWidget->widget(index));
        delete sva->getByteSource();
        delete sva;
    }
}

void GenericTab::onSearch(QByteArray item, QBitArray mask, bool)
{
    hexView->search(item, mask);
}

bool GenericTab::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) { // ctrl-[key]
            if (keyEvent->key() == Qt::Key_N)  {
                hexView->searchAgain();
                return true;
            } else if (keyEvent->key() == Qt::Key_G) {
                gotoWidget->setFocus();
                return true;
            } else if (keyEvent->key() == Qt::Key_F) {
                searchWidget->setFocus();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void GenericTab::dragEnterEvent(QDragEnterEvent *event)
{
    guiHelper->processDragEnter(event, byteSource);
}

void GenericTab::dropEvent(QDropEvent *event)
{
    guiHelper->processDropEvent(event, byteSource);
}

void GenericTab::integrateByteSource()
{
    roButton->refreshStateValue();

    ui->historyRedoPushButton->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
    ui->historyUndoPushButton->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
}

