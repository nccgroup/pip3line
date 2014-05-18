/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "transformwidget.h"
#include "ui_transformwidget.h"
#include "newbytedialog.h"
#include <transformabstract.h>
#include <transformmgmt.h>
#include <QComboBox>
#include <QTextStream>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QMenu>
#include <QClipboard>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTextCursor>
#include <QTableWidgetSelectionRange>
#include <QtAlgorithms>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QRgb>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QElapsedTimer>
#include <QMimeData>
#include <QValidator>
#include <QFileDialog>
#include <QTextCodec>
#include <threadedprocessor.h>
#include "sources/basicsource.h"
#include "crossplatform.h"
#include "shared/offsetgotowidget.h"
#include "shared/searchwidget.h"
#include "views/textview.h"
#include "views/hexview.h"
#include "shared/messagepanelwidget.h"
#include <QDebug>
#include <QRegExp>
#include "shared/clearallmarkingsbutton.h"

const int TransformWidget::MAX_DIRECTION_TEXT = 20;
const QString TransformWidget::NEW_BYTE_ACTION = "New Byte(s)";

TransformWidget::TransformWidget(GuiHelper *nguiHelper ,QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::TransformWidget();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::TransformWidget X{");
    }
    currentTransform = NULL;
    infoDialog = NULL;
    settingsTab = NULL;
    gotoWidget = NULL;
    searchWidget = NULL;
    guiHelper = nguiHelper;
    transformFactory = guiHelper->getTransformFactory();
    manager = guiHelper->getNetworkManager();
    logger = guiHelper->getLogger();
    byteSource = new(std::nothrow) BasicSource();
    if (byteSource == NULL) {
        qFatal("Cannot allocate memory for byteSource X{");
    }

    connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger, SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
    connect(byteSource, SIGNAL(updated(quintptr)), this, SLOT(refreshOutput()));
    connect(byteSource, SIGNAL(nameChanged(QString)), this, SIGNAL(tryNewName(QString)));

    ui->setupUi(this);

    buildSelectionArea();
    // cannot do that in the buildSelectionArea() function as it is called frequently
    ui->wayGroupBox->setVisible(false);
    ui->deleteButton->setEnabled(false);
    ui->infoPushButton->setEnabled(false);
    connect(ui->transfoComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTransformSelected(QString)), Qt::UniqueConnection);

    ui->activityStackedWidget->setCurrentIndex(1);
    ui->transfoComboBox->setFocusPolicy( Qt::StrongFocus );
    ui->transfoComboBox->installEventFilter(guiHelper) ;

    configureViewArea();

    messagePanel = new(std::nothrow) MessagePanelWidget(this);
    if (messagePanel == NULL) {
        qFatal("Cannot allocate memory for MessagePanelWidget X{");
    }


    ui->mainLayout->insertWidget(0,messagePanel);
    connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), messagePanel, SLOT(log(QString,QString,Pip3lineConst::LOGLEVEL)));

    firstView = true;
    setAcceptDrops(true);

    connect(transformFactory, SIGNAL(transformsUpdated()),this, SLOT(buildSelectionArea()), Qt::QueuedConnection);
    connect(guiHelper, SIGNAL(filterChanged()), this, SLOT(buildSelectionArea()));

    connect(this, SIGNAL(sendRequest(TransformRequest*)), guiHelper->getCentralTransProc(), SLOT(processRequest(TransformRequest*)), Qt::QueuedConnection);

 //   qDebug() << "Created" << this;
}

TransformWidget::~TransformWidget()
{

  //  qDebug() << "Destroying:" << this << " " << (currentTransform == NULL ? "Null" : currentTransform->name());

    clearCurrentTransform();
    delete gotoWidget;
    delete searchWidget;
    delete infoDialog;

    delete hexView;
    delete byteSource;
    logger = NULL;
    guiHelper = NULL;

    delete ui;

}

void TransformWidget::configureViewArea() {
    hexView = new(std::nothrow) HexView(byteSource,guiHelper,this);
    if (hexView == NULL) {
        qFatal("Cannot allocate memory for HexView X{");
    }

    textView = new(std::nothrow) TextView(byteSource,guiHelper,this);
    if (textView == NULL) {
        qFatal("Cannot allocate memory for TextView X{");
    }

    ui->tabWidget->addTab(textView,"Text");
    ui->tabWidget->addTab(hexView,"Hexadecimal");

    hexView->installEventFilter(this);
    textView->installEventFilter(this);
    ui->tabWidget->installEventFilter(this);
    this->installEventFilter(this);

    searchWidget = new(std::nothrow) SearchWidget(byteSource, this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }

    searchWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    searchWidget->setStopVisible(false);
    ui->toolsLayout->addWidget(searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,QBitArray,bool)), SLOT(onSearch(QByteArray,QBitArray,bool)));
    connect(textView, SIGNAL(searchStatus(bool)), searchWidget,SLOT(setError(bool)));

    gotoWidget = new(std::nothrow) OffsetGotoWidget(guiHelper, this);
    if (gotoWidget == NULL) {
        qFatal("Cannot allocate memory for OffsetGotoWidget X{");
    }

    gotoWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->toolsLayout->addWidget(gotoWidget);
    connect(gotoWidget, SIGNAL(gotoRequest(quint64,bool,bool,bool)), SLOT(onGotoOffset(quint64,bool,bool,bool)));

    clearAllMarkingsButton = new(std::nothrow) ClearAllMarkingsButton(byteSource,this);
    if (clearAllMarkingsButton == NULL) {
        qFatal("Cannot allocate memory for ClearAllMarkingsButton X{");
    }
    ui->uppperToolLayout->insertWidget(1,clearAllMarkingsButton);

    connect(ui->backwardPushButton, SIGNAL(clicked()), this, SLOT(onHistoryBackward()));
    connect(ui->forwardPushButton, SIGNAL(clicked()), this, SLOT(onHistoryForward()));

    connect(textView, SIGNAL(invalidText()), this, SLOT(onInvalidText()));
    connect(textView, SIGNAL(askForFileLoad()), this, SLOT(onFileLoadRequest()));
    connect(hexView, SIGNAL(askForFileLoad()), this, SLOT(onFileLoadRequest()));
}

void TransformWidget::buildSelectionArea() {
    QString currentTransformName;

    if (ui->transfoComboBox->currentIndex() != -1) {
        currentTransformName = ui->transfoComboBox->currentText();
    }
    //cleaning

    ui->transfoComboBox->blockSignals(true);
    ui->transfoComboBox->clear();

    guiHelper->buildTransformComboBox(ui->transfoComboBox, currentTransformName, true);
    ui->transfoComboBox->blockSignals(false);
}

void TransformWidget::clearCurrentTransform()
{
    if (currentTransform != NULL) {
        delete settingsTab;
        settingsTab = NULL;
        delete currentTransform;
        currentTransform = NULL;
        if (ui->tabWidget->count() > 2)
            ui->tabWidget->removeTab(2);
    }

    delete infoDialog;
    infoDialog = NULL;

}

void TransformWidget::updatingFromTransform() {
    configureDirectionBox();
    emit transformChanged();
    refreshOutput();
}

void TransformWidget::onHistoryBackward()
{
    byteSource->historyBackward();
}

void TransformWidget::onHistoryForward()
{
    byteSource->historyForward();
}

void TransformWidget::onTransformSelected(QString name) {
    clearCurrentTransform();

    currentTransform = transformFactory->getTransform(name);
    if (currentTransform != NULL) {
        currentTransform->setWay(TransformAbstract::INBOUND);
        integrateTransform();
        buildSelectionArea();
        emit transfoRequest(this);
    } else {
        QString mess = tr("Could not instantiate transformation :%1").arg(name);
        logger->logError(mess);
        QMessageBox::critical(this, tr("Error"), mess,QMessageBox::Ok);
    }
}

void TransformWidget::integrateTransform()
{
    if (currentTransform != NULL) {
        ui->descriptionLabel->setText(currentTransform->description());
        // need to grab the errors before getting the gui, in case the gui configuration generates some
        connect(currentTransform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString)));
        connect(currentTransform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString)));

        settingsTab = currentTransform->getGui(this);
        if (settingsTab != NULL) {
            ui->tabWidget->addTab(settingsTab, tr("Settings"));
        }

        configureDirectionBox();
        connect(currentTransform,SIGNAL(confUpdated()),this,SLOT(updatingFromTransform()), Qt::QueuedConnection);
        ui->deleteButton->setEnabled(true);
        ui->infoPushButton->setEnabled(true);
        emit transformChanged();
        refreshOutput();
    }
}

void TransformWidget::input(QByteArray inputdata) {
    byteSource->setData(inputdata);
    if (firstView) {
        if (byteSource->isReadableText()) {
            ui->tabWidget->setCurrentWidget(textView);
        }
        else {
            ui->tabWidget->setCurrentWidget(hexView);
        }
        firstView = false;
    }
}

QByteArray TransformWidget::output() {
    return outputData;
}

void TransformWidget::refreshOutput()
{
    if (currentTransform != NULL) {
        ui->activityStackedWidget->setCurrentIndex(0);

        TransformAbstract * ta = transformFactory->cloneTransform(currentTransform);
        if (ta != NULL) {
            TransformRequest *tr = new(std::nothrow) TransformRequest(
                        ta,
                        byteSource->getRawData(),
                        (quintptr) this);

            if (tr == NULL) {
                qFatal("Cannot allocate memory for TransformRequest X{");
            }

            connect(tr,SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(processingFinished(QByteArray,Messages)));
            emit sendRequest(tr);
        }
    }
}

void TransformWidget::processingFinished(QByteArray output, Messages messages)
{
    messagePanel->clear();
    outputData = output;

    if (messages.isEmpty()) {
        messagePanel->closeWidget();
    } else {
        for (int i = 0; i < messages.size() ; i++) {
            switch (messages.at(i).level) {
                case (LERROR):
                    logError(messages.at(i).message, messages.at(i).source);
                    break;
                case (LWARNING):
                    logWarning(messages.at(i).message, messages.at(i).source);
                    break;
                case (LSTATUS):
                    logStatus(messages.at(i).message, messages.at(i).source);
                    break;
                default:
                    qWarning("[TransformWidget::processingFinished] Unkown error level");
            }
        }
    }

    emit updated();
    ui->activityStackedWidget->setCurrentIndex(1);
}

void TransformWidget::on_encodeRadioButton_toggled(bool checked)
{
    if (checked && currentTransform != NULL) {
        currentTransform->setWay(TransformAbstract::INBOUND);
//        refreshOutput();
    }
}

void TransformWidget::on_decodeRadioButton_toggled(bool checked)
{
    if (checked && currentTransform != NULL) {
        currentTransform->setWay(TransformAbstract::OUTBOUND);
//        refreshOutput();
    }
}

void TransformWidget::updatingFrom() {
    TransformWidget* src = dynamic_cast<TransformWidget*>(sender());
    input(src->output());
}

TransformAbstract *TransformWidget::getTransform() {
    return currentTransform;
}

bool TransformWidget::setTransform(TransformAbstract * transf)  {
    if (transf != NULL) {
        clearCurrentTransform();

        currentTransform = transf;

        ui->transfoComboBox->blockSignals(true);
        ui->transfoComboBox->setCurrentIndex(ui->transfoComboBox->findText(currentTransform->name()));
        ui->transfoComboBox->blockSignals(false);

        integrateTransform();
        return true;
    }
    return false;
}

ByteSourceAbstract *TransformWidget::getSource()
{
    return byteSource;
}

ByteTableView *TransformWidget::getHexTableView()
{
    return hexView->getHexTableView();
}

void TransformWidget::forceUpdating()
{
    refreshOutput();
}

void TransformWidget::logWarning(const QString message, const QString source) {
    messagePanel->addMessage(message,Qt::blue);
    emit warning(message, source);
}

void TransformWidget::logError(const QString message, const QString source) {
    messagePanel->addMessage(message,Qt::red);
    emit error(message,source);
}

void TransformWidget::logStatus(const QString message, const QString source)
{
    messagePanel->addMessage(message,Qt::black);
    emit status(message,source);
}

void TransformWidget::reset()
{
    if (currentTransform != NULL) {
        clearCurrentTransform();

        ui->transfoComboBox->blockSignals(true);
        ui->transfoComboBox->setCurrentIndex(0);
        ui->transfoComboBox->blockSignals(false);
        ui->wayGroupBox->setVisible(false);
        delete settingsTab;
        settingsTab = NULL;
        ui->descriptionLabel->clear();
    }
    byteSource->clear();
    ui->deleteButton->setEnabled(false);
    ui->infoPushButton->setEnabled(false);
    ui->tabWidget->setCurrentWidget(textView);
}

void TransformWidget::configureDirectionBox()
{
    if (currentTransform->isTwoWays()) {
        if (currentTransform->way() == TransformAbstract::INBOUND) {
            ui->encodeRadioButton->setChecked(true);
        } else {
            ui->decodeRadioButton->setChecked(true);
        }
        QString tempText = currentTransform->inboundString();
        if (tempText.size() > MAX_DIRECTION_TEXT) {
            tempText = tempText.mid(0,MAX_DIRECTION_TEXT);
        }
        ui->encodeRadioButton->setText(tempText);
        tempText = currentTransform->outboundString();
        if (tempText.size() > MAX_DIRECTION_TEXT) {
            tempText = tempText.mid(0,MAX_DIRECTION_TEXT);
        }
        ui->decodeRadioButton->setText(tempText);

        ui->wayGroupBox->setEnabled(true);
        ui->wayGroupBox->setVisible(true);
    } else {
        ui->wayGroupBox->setEnabled(false);
        ui->wayGroupBox->setVisible(false);
    }
}

bool TransformWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_N && keyEvent->modifiers().testFlag(Qt::ControlModifier))  {
            if (ui->tabWidget->currentWidget() == textView) {
                textView->searchAgain();
            } else {
                hexView->searchAgain();
            }
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

void TransformWidget::updateView(quintptr)
{
    refreshOutput();
}

void TransformWidget::onInvalidText()
{
    ui->tabWidget->setCurrentWidget(hexView);
}

void TransformWidget::dragEnterEvent(QDragEnterEvent *event)
{
    guiHelper->processDragEnter(event, byteSource);
}

void TransformWidget::dropEvent(QDropEvent *event)
{
    guiHelper->processDropEvent(event, byteSource);
}

void TransformWidget::fromLocalFile(QString fileName)
{
    byteSource->fromLocalFile(fileName);
}

void TransformWidget::onFileLoadRequest()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
    if (!fileName.isEmpty()) {
        fromLocalFile(fileName);
    }
}

void TransformWidget::on_deleteButton_clicked()
{
    emit deletionRequest(this);
}

void TransformWidget::on_infoPushButton_clicked()
{
    if (currentTransform == NULL)
        return;

    if (infoDialog == NULL) {
        infoDialog = new(std::nothrow) InfoDialog(guiHelper, currentTransform,this);
        if (infoDialog == NULL) {
            qFatal("Cannot allocate memory for InfoDialog X{");
            return;
        }
    }
    infoDialog->setVisible(true);
}

void TransformWidget::on_clearDataPushButton_clicked()
{
    byteSource->clear();
}

void TransformWidget::onSearch(QByteArray item, QBitArray mask, bool couldBeText)
{
    if (ui->tabWidget->currentWidget() == textView && couldBeText) {
        textView->search(searchWidget->text().toUtf8());
    } else {
        ui->tabWidget->setCurrentWidget(hexView);
        hexView->search(item, mask);
    }
}

void TransformWidget::onGotoOffset(quint64 offset, bool absolute, bool negative, bool select)
{
    if (!hexView->goTo(offset,absolute,negative, select)) {
        gotoWidget->setStyleSheet(GuiStyles::LineEditError);
    } else {
        ui->tabWidget->setCurrentWidget(hexView);
        gotoWidget->setStyleSheet(qApp->styleSheet());
    }
}
