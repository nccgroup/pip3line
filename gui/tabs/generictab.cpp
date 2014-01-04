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
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QKeyEvent>
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../shared/readonlybutton.h"
#include "../shared/clearallmarkingsbutton.h"
#include "../shared/bytesourceguibutton.h"
#include "../shared/detachtabbutton.h"

const QString GenericTab::LOGID = "FileTab";

GenericTab::GenericTab(ByteSourceAbstract *bytesource, GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent)
{
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
    ui->mainLayout->insertWidget(1,hexView);

    searchWidget = new SearchWidget(byteSource, this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    ui->mainLayout->insertWidget(1,searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,bool)), SLOT(onSearch(QByteArray,bool)));

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

    connect(hexView,SIGNAL(askForFileLoad()), SLOT(fileLoadRequest()));
    connect(ui->loadPushButton, SIGNAL(clicked()), SLOT(fileLoadRequest()));

    connect(ui->historyUndoPushButton, SIGNAL(clicked()), SLOT(onHistoryBackward()));
    connect(ui->historyRedoPushButton, SIGNAL(clicked()), SLOT(onHistoryForward()));
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
        logger->logError("Empty file name, ignoring",LOGID);
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
        int ret = QMessageBox::warning(this, tr("Overwritting the entire source"), tr("This will overwrite the entire source"),QMessageBox::Abort, QMessageBox::Ok);
        if (ret == QMessageBox::Ok) {
            byteSource->setData(data);
        }
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_RESET capability").arg(((QObject *)byteSource)->metaObject()->className()),QMessageBox::Ok);
    }
}

bool GenericTab::canReceiveData()
{
    return false;
}

void GenericTab::fileLoadRequest()
{
    QString fileName;
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_LOADFILE)) {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            byteSource->fromLocalFile(fileName);
            integrateByteSource();
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

void GenericTab::onSearch(QByteArray item, bool)
{
    hexView->search(item);
}

bool GenericTab::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_N && keyEvent->modifiers().testFlag(Qt::ControlModifier))  {
            hexView->searchAgain();
            return true;
        } else if (keyEvent->key() == Qt::Key_G && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            gotoWidget->setFocus();
            return true;
        } else if (keyEvent->key() == Qt::Key_F && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            searchWidget->setFocus();
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void GenericTab::integrateByteSource()
{
    roButton->refreshStateValue();

    ui->historyRedoPushButton->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
    ui->historyUndoPushButton->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
}

