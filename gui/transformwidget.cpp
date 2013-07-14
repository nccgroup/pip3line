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
#include <QMimeData>
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
#include "../tools/transformrequest.h"
#include "sources/basicsource.h"
#include "crossplatform.h"
#include <QDebug>
#include <QRegExp>

OffsetValidator::OffsetValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State OffsetValidator::validate(QString &input, int &) const
{
    if (input.isEmpty())
        return QValidator::Intermediate;

    QRegExp offsetRegExp("^([=+-])?([oxn])?([0-9a-fA-F]{0,20})$");

    if (offsetRegExp.indexIn(input) != -1) {

        if (!offsetRegExp.cap(3).isEmpty()) {
            quint64 val = 0;
            bool ok = false;
            if (offsetRegExp.cap(2) == "o") {
                val = offsetRegExp.cap(3).toULongLong(&ok, 8);
            } else if (offsetRegExp.cap(2) == "n") {
                val = offsetRegExp.cap(3).toULongLong(&ok, 10);
            } else {
                val = offsetRegExp.cap(3).toULongLong(&ok, 16);
            }

            if (ok && val < LONG_LONG_MAX) {
                return QValidator::Acceptable;
            } else {
                return QValidator::Invalid;
            }

        }
        return QValidator::Intermediate;
    }
    return QValidator::Invalid;
}

//=======================================================================

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
    guiHelper = nguiHelper;
    transformFactory = guiHelper->getTransformFactory();
    manager = guiHelper->getNetworkManager();
    logger = guiHelper->getLogger();
    byteSource = new(std::nothrow) BasicSource();
    if (byteSource == NULL) {
        qFatal("Cannot allocate memory for byteSource X{");
    }

    connect(byteSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), logger, SLOT(logMessage(QString,QString,Pip3lineConst::LOGLEVEL)));
    connect(byteSource, SIGNAL(updated(quintptr)), this, SLOT(refreshOutput()));

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

    ui->messagesScrollArea->hide();
    firstView = true;
    setAcceptDrops(true);

    connect(transformFactory, SIGNAL(transformsUpdated()),this, SLOT(buildSelectionArea()), Qt::QueuedConnection);
    connect(guiHelper, SIGNAL(filterChanged()), this, SLOT(buildSelectionArea()));

    qDebug() << "Created" << this;
}

TransformWidget::~TransformWidget()
{

    qDebug() << "Destroying:" << this << " " << (currentTransform == NULL ? "Null" : currentTransform->name());

    clearCurrentTransform();

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


    ui->clearMarkingsPushButton->setEnabled(false);
    connect(ui->clearMarkingsPushButton, SIGNAL(clicked()), hexView, SLOT(onClearAllMArkings()));

    OffsetValidator *val = new OffsetValidator(ui->gotoLineEdit);
    ui->gotoLineEdit->setValidator(val);
    ui->gotoLineEdit->installEventFilter(this);
    ui->searchLineEdit->installEventFilter(this);
    hexView->installEventFilter(this);
    textView->installEventFilter(this);
    ui->tabWidget->installEventFilter(this);
    this->installEventFilter(this);

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

        settingsTab = currentTransform->getGui(this);
        if (settingsTab != NULL) {
            ui->tabWidget->addTab(settingsTab, tr("Settings"));
        }

        configureDirectionBox();
        connect(currentTransform,SIGNAL(confUpdated()),this,SLOT(updatingFromTransform()), Qt::QueuedConnection);
        connect(currentTransform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString)));
        connect(currentTransform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString)));

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

        TransformAbstract * ta = transformFactory->loadTransformFromConf(currentTransform->getConfiguration());
        if (ta != NULL) {
            TransformRequest *tr = new TransformRequest(
                        ta,
                        byteSource->getRawData(),
                        (quintptr) this);

            connect(tr,SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(processingFinished(QByteArray,Messages)));
            guiHelper->processTransform(tr);
        }
    }
}

void TransformWidget::processingFinished(QByteArray output, Messages messages)
{
    ui->messagesDisplay->clear();
    outputData = output;

    if (messages.isEmpty()) {
        ui->messagesScrollArea->hide();
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
        refreshOutput();
    }
}

void TransformWidget::on_decodeRadioButton_toggled(bool checked)
{
    if (checked && currentTransform != NULL) {
        currentTransform->setWay(TransformAbstract::OUTBOUND);
        refreshOutput();
    }
}

void TransformWidget::updatingFrom() {
    TransformWidget* src = static_cast<TransformWidget*>(sender());
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

ByteSourceAbstract *TransformWidget::getBytes() const
{
    return byteSource;
}

void TransformWidget::forceUpdating()
{
    refreshOutput();
}

void TransformWidget::logWarning(const QString message, const QString source) {
    addMessage(message,Qt::blue);
    emit warning(message, source);
}

void TransformWidget::logError(const QString message, const QString source) {
    addMessage(message,Qt::red);
    emit error(message,source);
}

void TransformWidget::logStatus(const QString message, const QString source)
{
    addMessage(message,Qt::black);
    emit status(message,source);
}

void TransformWidget::addMessage(const QString &message, QColor color)
{
    ui->messagesDisplay->setTextColor(color);
    ui->messagesDisplay->append(message);
    ui->messagesScrollArea->setVisible(true);
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

void TransformWidget::setDownload(QUrl url)
{
    if (manager != NULL) {
        DownloadManager * downloadManager = new(std::nothrow) DownloadManager(url, manager, byteSource);
        if (downloadManager == NULL) {
            qFatal("Cannot allocate memory for setDownload downloadManager X{");
            return;
        }
        connect(downloadManager, SIGNAL(error(QString,QString)), this, SLOT(logError(QString, QString)));
        connect(downloadManager, SIGNAL(error(QString,QString)), this, SLOT(logWarning(QString, QString)));
        connect(downloadManager,SIGNAL(finished(DownloadManager *)),this, SLOT(downloadFinished(DownloadManager *)));
        downloadManager->launch();
        ui->activityStackedWidget->setCurrentIndex(0);

    } else {
        logError(tr("No NetworkManager configured, no download launched"));
    }
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
        if (obj == ui->gotoLineEdit && keyEvent->key() == Qt::Key_Return) {
            onGoToOffset(keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            return true;
        } else if (obj == ui->searchLineEdit && keyEvent->key() == Qt::Key_Return) {
            int modifiers = keyEvent->modifiers();
            if ((modifiers & Qt::ShiftModifier) || (modifiers & Qt::AltModifier) || (modifiers & Qt::ControlModifier))
                ui->tabWidget->setCurrentWidget(hexView);
            onSearch(modifiers);
        } else if (keyEvent->key() == Qt::Key_G && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            ui->gotoLineEdit->setFocus();
            ui->gotoLineEdit->selectAll();
        } else if (keyEvent->key() == Qt::Key_F && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            ui->searchLineEdit->setFocus();
            ui->searchLineEdit->selectAll();
        } else if (keyEvent->key() == Qt::Key_N && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            onSearch(0);
        }
    }
    return QObject::eventFilter(obj, event);
}

void TransformWidget::downloadFinished(DownloadManager * downloadManager)
{
    ui->activityStackedWidget->setCurrentIndex(1);
    delete downloadManager;
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
    if (event->mimeData()->hasHtml() || event->mimeData()->hasText() || event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void TransformWidget::dropEvent(QDropEvent *event)
{
    QStringList formats  = event->mimeData()->formats();

    for (int i = 0; i < formats.size(); i++) {
        qDebug() << formats.at(i);
    }

    if (event->mimeData()->hasImage()) {
            qDebug() << event->mimeData()->imageData();
    } else if (event->mimeData()->hasUrls() || formats.contains("text/uri-list")) {

        QList<QUrl> list = event->mimeData()->urls();


        if (list.size() < 1) {
            logError(tr("Url list is empty, nothing to drop"));
            event->acceptProposedAction();
            return;
        }

        if (list.size() > 1) {
            logError(tr("Multiple urls entered, only opening the first one"));
        }

        QUrl resource = list.at(0);

        QString filename = QString(QByteArray::fromPercentEncoding(resource.toEncoded()));

        logger->logStatus(tr("Received %1 for Drop action").arg(filename));

        if (resource.scheme() == "file") {
#if defined(Q_OS_WIN32)
            if (filename.at(0) == '/') // bug on QT < 5 (Windows only)
                filename = filename.mid(1);
            if (filename.startsWith("file:///"))
                filename = filename.mid(8);
#else
            if (filename.startsWith("file:///"))
                filename = filename.mid(7);
#endif

            fromLocalFile(filename);
        } else {
            setDownload(resource);
        }
        event->acceptProposedAction();
    } else if (event->mimeData()->hasText() || event->mimeData()->hasHtml()) {
        QUrl resource(event->mimeData()->text().toUtf8());

        if (resource.scheme() ==  "https" || resource.scheme() ==  "http" || resource.scheme() ==  "ftp" || resource.scheme() ==  "file") {
            int res = QMessageBox::question(this, "Download?","This looks like a valid URL.", QMessageBox::Yes, QMessageBox::No);
            if (res ==  QMessageBox::Yes) {
                logger->logStatus(tr("Received %1 for Drop action").arg(resource.toString()));

                setDownload(resource);

            } else {
                byteSource->setData(event->mimeData()->text().toUtf8());
            }
        } else {
            byteSource->setData(event->mimeData()->text().toUtf8());

        }
        event->acceptProposedAction();
    } else {
        logger->logWarning(tr("Don't know how to handle this Drop action"));
    }
}

void TransformWidget::fromLocalFile(QString fileName)
{
    QFileInfo finfo(fileName);
    emit tryNewName(finfo.fileName());

    byteSource->fromLocalFile(fileName);
}

void TransformWidget::onFileLoadRequest()
{
    QString fileName;
    if (byteSource->hasCapability(ByteSourceAbstract::CAP_RESET)) {
        fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
        if (!fileName.isEmpty()) {
            fromLocalFile(fileName);
        }
    } else {
        QMessageBox::critical(this,tr("Error"), tr("%1 does not have the CAP_RESET capability, ignoring").arg(((QObject *)byteSource)->metaObject()->className()),QMessageBox::Ok);
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
        infoDialog = new(std::nothrow) InfoDialog(currentTransform,this);
        if (infoDialog == NULL) {
            qFatal("Cannot allocate memory for InfoDialog X{");
            return;
        }
    }
    infoDialog->setVisible(true);
}

void TransformWidget::on_clearMarkingsPushButton_clicked()
{
    ui->clearMarkingsPushButton->setEnabled(false);
}

void TransformWidget::on_clearDataPushButton_clicked()
{
    byteSource->clear();
}

void TransformWidget::onGoToOffset(bool select)
{
    QString gotoValue = ui->gotoLineEdit->text();
    if (gotoValue.isEmpty()) {
        return;
    }

    QRegExp offsetRegExp("^([=+-])?([oxn])?([0-9a-fA-F]{0,20})$");

    if (offsetRegExp.indexIn(gotoValue) != -1) {


        if (!offsetRegExp.cap(3).isEmpty()) {
            quint64 val = 0;
            bool ok = false;
            if (offsetRegExp.cap(2) == "o") {
                val = offsetRegExp.cap(3).toULongLong(&ok, 8);
            } else if (offsetRegExp.cap(2) == "n") {
                val = offsetRegExp.cap(3).toULongLong(&ok, 10);
            } else if (offsetRegExp.cap(2) == "x"){
                val = offsetRegExp.cap(3).toULongLong(&ok, 16);
            } else { // default from configuration
                val = offsetRegExp.cap(3).toULongLong(&ok, guiHelper->getDefaultOffsetBase());
            }


            if (ok && val < LONG_LONG_MAX) {
                ok = false;
                if (offsetRegExp.cap(1) == "-") {
                    ok = hexView->goTo(-1 * val,false,select);
                } else if (offsetRegExp.cap(1) == "+") {
                    ok = hexView->goTo(val,false,select);
                } else {
                    ok = hexView->goTo(val,true,select);
                }

                ui->tabWidget->setCurrentWidget(hexView);

                if (!ok) {
                    ui->gotoLineEdit->setStyleSheet(GuiStyles::LineEditError);
                } else {
                   ui->gotoLineEdit->setStyleSheet("");
                }
            } else {
                logger->logError("Invalid Offset value T_T");
            }

        }
    } else {
        logger->logError("Invalid Offset specification T_T");
    }
}

void TransformWidget::onSearch(int modifiers)
{

    if (ui->tabWidget->currentWidget() == textView) {
        if (textView->search(ui->searchLineEdit->text())) {
            ui->searchLineEdit->setStyleSheet("");
        } else {
            ui->searchLineEdit->setStyleSheet(GuiStyles::LineEditError);
        }
    } else {
        QByteArray item = ui->searchLineEdit->text().toUtf8();
        bool errorHasOccured = false;
        if (!item.isEmpty()) {
            if ((modifiers & Qt::ControlModifier) || (modifiers & Qt::AltModifier)) {

                if (modifiers & Qt::ControlModifier) {
                    QTextCodec *codec = NULL;
                    codec = QTextCodec::codecForName("UTF-16");
                    if (codec == NULL) {
                        logger->logError(tr("Could not find the require codec T_T"));
                        return;
                    } else {
                        QTextEncoder *encoder = codec->makeEncoder(QTextCodec::IgnoreHeader | QTextCodec::ConvertInvalidToNull);
                        item = encoder->fromUnicode(ui->searchLineEdit->text());
                        errorHasOccured = encoder->hasFailure();
                        delete encoder;

                    }
                } // if utf-8 required, there is nothing to do

            } else {
                item = QByteArray::fromHex(item);
            }

            if (item.isEmpty()) {
                errorHasOccured = true;
            } else {
                if (hexView->search(item)) {
                    ui->tabWidget->setCurrentWidget(hexView);
                } else {
                    errorHasOccured = true;
                }
            }

            if (errorHasOccured)
                ui->searchLineEdit->setStyleSheet(GuiStyles::LineEditError);
            else
                ui->searchLineEdit->setStyleSheet("");
        }
    }
}
