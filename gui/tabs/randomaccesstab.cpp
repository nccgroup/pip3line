/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "randomaccesstab.h"
#include "ui_randomaccesstab.h"
#include "../shared/offsetgotowidget.h"
#include "../shared/searchwidget.h"
#include "../sources/bytesourceabstract.h"
#include "../views/hexview.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../shared/readonlybutton.h"
#include "../shared/clearallmarkingsbutton.h"
#include "../shared/bytesourceguibutton.h"
#include "../shared/detachtabbutton.h"
#include "../views/bytetableview.h"

const QString RandomAccessTab::LOGID = "RandomAccessTab";

RandomAccessTab::RandomAccessTab(ByteSourceAbstract *nbyteSource, GuiHelper *guiHelper, QWidget *parent) :
    TabAbstract(guiHelper,parent)
{
    ui = new(std::nothrow) Ui::RandomAccessTab();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::RandomAccessTab X{");
    }
    ui->setupUi(this);

    byteSource = nbyteSource;
    setName(byteSource->name());
    connect(byteSource,SIGNAL(nameChanged(QString)), SLOT(setName(QString)));

    hexView = new(std::nothrow) HexView(byteSource, guiHelper,this);
    if (hexView == NULL) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    ui->mainLayout->insertWidget(ui->mainLayout->indexOf(ui->logsWidget) + 1,hexView);

    searchWidget = new(std::nothrow) SearchWidget(byteSource, this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }

    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    ui->mainLayout->insertWidget( 1,searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,QBitArray,bool)), SLOT(onSearch(QByteArray,QBitArray,bool)));

    hexView->installEventFilter(this);
    hexView->getHexTableView()->verticalScrollBar()->setVisible(true);
    connect(hexView,SIGNAL(askForFileLoad()), SLOT(fileLoadRequest()));
    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper,this);
    if (gotoWidget == NULL) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }
    gotoWidget->setMaximumWidth(150);
    ui->toolsLayout->insertWidget(0,gotoWidget);
    connect(gotoWidget,SIGNAL(gotoRequest(quint64,bool,bool,bool)), SLOT(onGotoOffset(quint64,bool,bool,bool)));

    roButton = new(std::nothrow) ReadOnlyButton(byteSource,this);
    if (roButton == NULL) {
        qFatal("Cannot allocate memory for ReadOnlyButton X{");
    }
    ui->toolsLayout->insertWidget(2,roButton);

    clearAllMarksButton = new(std::nothrow) ClearAllMarkingsButton(byteSource,this);
    if (clearAllMarksButton == NULL) {
        qFatal("Cannot allocate memory for clearAllMarkingsButton X{");
    }
    ui->toolsLayout->insertWidget(2,clearAllMarksButton);

    guiButton = new(std::nothrow) ByteSourceGuiButton(byteSource,guiHelper,this);
    if (guiButton == NULL) {
        qFatal("Cannot allocate memory for ByteSourceGuiButton X{");
    }

    ui->toolsLayout->insertWidget(3, guiButton);

    if (byteSource->hasDiscreetView()) {
        ui->viewSizeSpinBox->setValue(byteSource->viewSize());
        connect(ui->viewSizeSpinBox, SIGNAL(valueChanged(int)),byteSource, SLOT(setViewSize(int)));
    } else {
        ui->viewSizeSpinBox->setVisible(false);
    }
    ui->logsWidget->setVisible(false);

    detachButton = new(std::nothrow) DetachTabButton(this);
    if (detachButton == NULL) {
        qFatal("Cannot allocate memory for detachButton X{");
    }

    ui->toolsLayout->insertWidget(0,detachButton);

    connect(ui->prevPushButton, SIGNAL(clicked()), byteSource,SLOT(historyBackward()));
    connect(ui->nextPushButton, SIGNAL(clicked()), byteSource, SLOT(historyForward()));
    connect(ui->closeLogsPushButton, SIGNAL(clicked()), SLOT(onCloseLogView()));
}

RandomAccessTab::~RandomAccessTab()
{
    delete gotoWidget;
    delete searchWidget;
    delete roButton;
    delete byteSource;
    delete ui;
}

int RandomAccessTab::getBlockCount() const
{
    return 1;
}

ByteSourceAbstract *RandomAccessTab::getSource(int)
{
    return byteSource;
}

ByteTableView *RandomAccessTab::getHexTableView(int)
{
    return hexView->getHexTableView();
}

void RandomAccessTab::loadFromFile(QString fileName)
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

void RandomAccessTab::setData(const QByteArray &)
{
    QMessageBox::critical(this,tr("Action rejected T_T"), tr("Cannot set data in this tab"),QMessageBox::Ok);
}

bool RandomAccessTab::canReceiveData()
{
    return false;
}

void RandomAccessTab::fileLoadRequest()
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

void RandomAccessTab::integrateByteSource()
{
    roButton->refreshStateValue();

    ui->prevPushButton->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
    ui->nextPushButton->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_HISTORY));
}

void RandomAccessTab::onSearch(QByteArray item, QBitArray mask, bool)
{
    hexView->search(item, mask);
}

void RandomAccessTab::onGotoOffset(quint64 offset, bool absolute,bool negative, bool select)
{
    if (!hexView->goTo(offset,absolute,negative,select)) {
        gotoWidget->setStyleSheet(GuiStyles::LineEditError);
    } else {
        gotoWidget->setStyleSheet(qApp->styleSheet());
    }
}

void RandomAccessTab::log(QString mess, QString , Pip3lineConst::LOGLEVEL level)
{
    QColor color = Qt::black;
    if (level == Pip3lineConst::LERROR)
        color = Qt::red;
    else if (level == Pip3lineConst::LWARNING)
        color = Qt::blue;

    ui->logsTextEdit->setTextColor(color);
    ui->logsTextEdit->append(mess);
    ui->logsWidget->setVisible(true);
}

void RandomAccessTab::onCloseLogView()
{
    ui->logsWidget->hide();
    ui->logsTextEdit->clear();
}

bool RandomAccessTab::eventFilter(QObject *obj, QEvent *event)
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
