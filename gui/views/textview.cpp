/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "textview.h"
#include "ui_textview.h"
#include <QFont>
#include <QMimeData>
#include <QScrollBar>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QTextEncoder>
#include <QTextDecoder>
#include <QMenu>
#include <QAction>
#include "../tabs/tababstract.h"
#include "../sources/bytesourceabstract.h"
#include "../loggerwidget.h"
#include "../guihelper.h"

// We need to set this limit as QTextEdit has difficulties with large input
const int TextView::MAX_TEXT_VIEW = 100000;
const QString TextView::DEFAULT_CODEC = "UTF-8";
const QString TextView::LOGID = "TextView";

TextView::TextView(ByteSourceAbstract *nbyteSource, GuiHelper *nguiHelper, QWidget *parent) :
    SingleViewAbstract(nbyteSource, nguiHelper, parent)
{
    connect(byteSource,SIGNAL(updated(quintptr)), this, SLOT(updateText(quintptr)), Qt::UniqueConnection);
    ui = new(std::nothrow) Ui::TextView();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::TextView X{");
    }
    globalContextMenu = NULL;
    sendToMenu = NULL;
    loadMenu = NULL;
    copyMenu = NULL;
    loadFileAction = NULL;
    saveToFileAction = NULL;
    sendToNewTabAction = NULL;
    selectAllAction = NULL;
    keepOnlySelectedAction = NULL;
    currentCodec = NULL;
    errorNotReported = true;
    ui->setupUi(this);

    ui->plainTextEdit->installEventFilter(this);
    connect(ui->plainTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(ui->plainTextEdit,SIGNAL(selectionChanged()), this, SLOT(updateStats()));
    connect(guiHelper, SIGNAL(tabsUpdated()), this, SLOT(updateSendToMenu()));
    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportMenu()));
    setAcceptDrops(true);
    QFont textViewFont;
    textViewFont.setFamily("Monospace");
    textViewFont.setPointSize(10);
    ui->plainTextEdit->setFont(textViewFont);

    QList<QByteArray> codecs =  QTextCodec::availableCodecs();
    qSort(codecs);
    for (int i = 0; i < codecs.size(); i++) {
        ui->codecsComboBox->addItem(QString(codecs.at(i)),QVariant(codecs.at(i)));
    }
    ui->codecsComboBox->setCurrentIndex(ui->codecsComboBox->findData(DEFAULT_CODEC));
    ui->codecsComboBox->installEventFilter(guiHelper);
    connect(ui->codecsComboBox,SIGNAL(currentIndexChanged(QString)), this, SLOT(onCodecChange(QString)));

    onCodecChange(DEFAULT_CODEC);
    buildContextMenu();
}

TextView::~TextView()
{
    delete sendToMenu;
    delete saveToFileAction;
    delete loadMenu;
    delete copyMenu;
    delete loadFileAction;
    delete selectAllAction;
    delete keepOnlySelectedAction;
    delete globalContextMenu;
    delete ui;
    logger = NULL;
    guiHelper = NULL;
}

void TextView::setModel(ByteSourceAbstract *dataModel)
{
    byteSource = dataModel;
    connect(byteSource,SIGNAL(updated(quintptr)), this, SLOT(updateText(quintptr)), Qt::UniqueConnection);
}

void TextView::search(QByteArray block)
{
    QString item = QString::fromUtf8(block.data(), block.size());
    previousSearch = block;
    bool found = ui->plainTextEdit->find(item);
    if (!found) {
        QTextCursor cursor(ui->plainTextEdit->textCursor());
        cursor.movePosition(QTextCursor::Start);
        ui->plainTextEdit->setTextCursor(cursor);
        found = ui->plainTextEdit->find(item);
    }

    emit searchStatus(!found);
}

void TextView::onRightClick(QPoint pos)
{
    // if any selection
    bool hasTextSelection = ui->plainTextEdit->textCursor().hasSelection();
    sendToMenu->setEnabled(hasTextSelection);
    copyMenu->setEnabled(hasTextSelection);
    keepOnlySelectedAction->setEnabled(hasTextSelection);
    if (ui->plainTextEdit->toPlainText().isEmpty()) {
        saveToFileAction->setEnabled(false);
        selectAllAction->setEnabled(false);
    } else {
        saveToFileAction->setEnabled(true);
        selectAllAction->setEnabled(true);
    }
    globalContextMenu->exec(this->mapToGlobal(pos));
}

void TextView::onLoad(QAction * action)
{
    guiHelper->loadAction(action->text(), byteSource);
}

void TextView::onCopy(QAction *action)
{
    guiHelper->copyAction(action->text(), encode(ui->plainTextEdit->textCursor().selection().toPlainText()));
}

void TextView::onSaveToFile()
{
    if (ui->plainTextEdit->textCursor().hasSelection()) {
        guiHelper->saveToFileAction(encode(ui->plainTextEdit->textCursor().selection().toPlainText()),this);
    }
    else
        guiHelper->saveToFileAction(byteSource->getRawData(),this);
}

void TextView::onLoadFile()
{
    emit askForFileLoad();
}

void TextView::onSendToTab(QAction * action)
{
    QByteArray dataToSend = encode(ui->plainTextEdit->textCursor().selection().toPlainText());

    if (action == sendToNewTabAction) {
        guiHelper->sendToNewTab(dataToSend);
    } else {
        if (sendToTabMapping.contains(action)) {
            TabAbstract * tg = sendToTabMapping.value(action);
            tg->setData(dataToSend);
            tg->bringFront();
        } else {
            logger->logError(tr("Tab not found for sending"));
        }
    }
}

void TextView::onSelectAll()
{
    ui->plainTextEdit->selectAll();
}

void TextView::onKeepOnlySelection()
{
    QString newText = ui->plainTextEdit->textCursor().selection().toPlainText();
    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(newText);
}

void TextView::onCodecChange(QString codecName)
{
    QTextCodec *codec = QTextCodec::codecForName(codecName.toUtf8());
    if (codec == NULL) {
       logger->logError(tr("Cannot found the text codec: %1. Ignoring request.").arg(codecName),LOGID);
    } else {
        currentCodec = codec;
        updateText(0);
    }
}

void TextView::updateSendToMenu()
{
    sendToMenu->clear(); // action created on the fly should be automatically deleted
    sendToTabMapping.clear(); // clearing mapping

    sendToMenu->addAction(sendToNewTabAction);
    sendToMenu->addSeparator();

    QList<TabAbstract *> list = guiHelper->getTabs();
    for (int i = 0; i < list.size(); i++) {
        QAction * action = new(std::nothrow) QAction(list.at(i)->getName(),sendToMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateSendToMenu X{");
            return;
        }
        sendToTabMapping.insert(action, list.at(i));
        sendToMenu->addAction(action);
    }
}

void TextView::updateImportExportMenu()
{
    guiHelper->updateLoadContextMenu(loadMenu);
    guiHelper->updateCopyContextMenu(copyMenu);
}

void TextView::buildContextMenu()
{
    ui->plainTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->plainTextEdit,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClick(QPoint)));

    sendToMenu = new(std::nothrow) QMenu(tr("Send selection to"));
    if (sendToMenu == NULL) {
        qFatal("Cannot allocate memory for sendToMenu X{");
        return;
    }
    sendToNewTabAction = new(std::nothrow) QAction(GuiHelper::SEND_TO_NEW_TAB_ACTION, this);
    if (sendToNewTabAction == NULL) {
        qFatal("Cannot allocate memory for sendToNewTab X{");
    }

    updateSendToMenu();
    connect(sendToMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSendToTab(QAction*)), Qt::UniqueConnection);

    loadMenu = new(std::nothrow) QMenu(tr("Load from clipboard"));
    if (loadMenu == NULL) {
        qFatal("Cannot allocate memory for importMenu X{");
        return;
    }
    guiHelper->updateLoadContextMenu(loadMenu);
    connect(loadMenu, SIGNAL(triggered(QAction*)), this, SLOT(onLoad(QAction*)), Qt::UniqueConnection);

    copyMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyMenu == NULL) {
        qFatal("Cannot allocate memory for copyMenu X{");
        return;
    }

    guiHelper->updateCopyContextMenu(copyMenu);
    connect(copyMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopy(QAction*)), Qt::UniqueConnection);

    saveToFileAction = new(std::nothrow) QAction("Save to file", this);
    if (saveToFileAction == NULL) {
        qFatal("Cannot allocate memory for saveToFileAction X{");
    }
    connect(saveToFileAction, SIGNAL(triggered()), this, SLOT(onSaveToFile()));

    loadFileAction = new(std::nothrow) QAction("Load from file", this);
    if (loadFileAction == NULL) {
        qFatal("Cannot allocate memory for loadFileAction X{");
    }
    connect(loadFileAction, SIGNAL(triggered()), this, SLOT(onLoadFile()));

    selectAllAction = new(std::nothrow) QAction("Select all", this);
    if (selectAllAction == NULL) {
        qFatal("Cannot allocate memory for selectAllAction X{");
    }
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(onSelectAll()));

    keepOnlySelectedAction = new(std::nothrow) QAction("Keep only selected", this);
    if (keepOnlySelectedAction == NULL) {
        qFatal("Cannot allocate memory for keepOnlySelectedAction X{");
    }

    connect(keepOnlySelectedAction, SIGNAL(triggered()), this, SLOT(onKeepOnlySelection()));

    globalContextMenu = new(std::nothrow) QMenu();

    if (globalContextMenu == NULL) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
        return;
    }

    globalContextMenu->addAction(selectAllAction);
    globalContextMenu->addAction(keepOnlySelectedAction);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(loadFileAction);
    globalContextMenu->addMenu(loadMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addMenu(copyMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(saveToFileAction);

}

void TextView::onTextChanged()
{
    byteSource->setData(encode(ui->plainTextEdit->toPlainText()),(quintptr) this);
    updateStats();
}

void TextView::updateText(quintptr source)
{
    if (source == (quintptr) this)
        return;

    ui->plainTextEdit->blockSignals(true);
    ui->plainTextEdit->clear();
    if (byteSource->size() > (quint64)MAX_TEXT_VIEW) {
        ui->plainTextEdit->appendPlainText("Data Too large for this view");
        ui->plainTextEdit->blockSignals(false);
        ui->plainTextEdit->setEnabled(false);
        ui->statsLabel->setText(QString(""));
        ui->codecsComboBox->setStyleSheet("");
        emit invalidText();
    } else {
        if (byteSource->size() > 0) {
            if (currentCodec != NULL) { //safeguard
                QTextDecoder *decoder = currentCodec->makeDecoder(QTextCodec::ConvertInvalidToNull | QTextCodec::IgnoreHeader);
                QString textf = decoder->toUnicode(byteSource->getRawData().constData(),byteSource->size());
                if (decoder->hasFailure()) {
                    if (errorNotReported) {
                        logger->logError(tr("invalid text decoding [%1]").arg(QString::fromUtf8(currentCodec->name())),LOGID);
                        ui->codecsComboBox->setStyleSheet(GuiStyles::ComboBoxError);
                        errorNotReported = false;
                    }
                } else {
                    ui->codecsComboBox->setStyleSheet("");
                    errorNotReported = true;
                }
                delete decoder;
                ui->plainTextEdit->appendPlainText(textf);
                updateStats();
                ui->plainTextEdit->moveCursor(QTextCursor::Start);
                ui->plainTextEdit->ensureCursorVisible();
                ui->plainTextEdit->setEnabled(true);
            } else {
                logger->logError(tr(":updatedText() currentCodec is NULL T_T"),LOGID);
            }
        } else {

            ui->plainTextEdit->setEnabled(true);
        }
    }
    ui->plainTextEdit->blockSignals(false);
}


void TextView::reveceivingTextChunk(const QString &chunk)
{
    ui->plainTextEdit->insertPlainText(chunk);
}

void TextView::updateStats()
{
    if (ui->plainTextEdit->isEnabled()) {
        QString plainText = ui->plainTextEdit->toPlainText();
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        int size = 0;
        if (cursor.hasSelection()){
            size = cursor.selectionEnd() - cursor.selectionStart();
            // that should not be here but that's just easier as updateStats is
            // called everytime the selection change anyway
            guiHelper->sendNewSelection(encode(cursor.selection().toPlainText()));
        }
        // updating text info
        QString ret = "Size: ";
        ret.append(QString::number(plainText.size())).append(tr(" characters"));
        if (size > 0)
            ret.append(tr(" (%1 selected) |").arg(size));
        ret.append(tr(" Lines: ")).append(QString::number(plainText.count("\n") + 1));

        ui->statsLabel->setText(ret);
    }
}


bool TextView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->plainTextEdit) {
        ui->plainTextEdit->setAttribute(Qt::WA_NoMousePropagation, false);
        if (event->type() == QEvent::KeyPress) {
             QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
             switch (keyEvent->key())
             {
                 case Qt::Key_Z:
                     if (keyEvent->modifiers().testFlag(Qt::ShiftModifier) && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                         byteSource->historyForward();
                         keyEvent->accept();
                         return true;
                     }
                     else if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
                         byteSource->historyBackward();
                         keyEvent->accept();
                         return true;
                     }
                    break;
                 default:
                     return false;
             }

        } else if (event->type() == QEvent::Wheel) {
            if (ui->plainTextEdit->verticalScrollBar()->isVisible()) {
                ui->plainTextEdit->setAttribute(Qt::WA_NoMousePropagation);
            }

            return false;
        }
    }

    return QWidget::eventFilter(obj, event);

}

QByteArray TextView::encode(QString text)
{
    QByteArray ret;
    if (currentCodec != NULL) { // safeguard
        QTextEncoder *encoder = currentCodec->makeEncoder(QTextCodec::ConvertInvalidToNull | QTextCodec::IgnoreHeader);

        ret = encoder->fromUnicode(text);
        if (encoder->hasFailure()) {
            logger->logError(tr("Some error(s) occured during the encoding process [%1]").arg(QString::fromUtf8(currentCodec->name())),LOGID);
            ui->codecsComboBox->setStyleSheet(GuiStyles::ComboBoxError);
        } else {
            ui->codecsComboBox->setStyleSheet(qApp->styleSheet());
        }
        delete encoder;
    }  else {
        logger->logError(tr(":encode() currentCodec is NULL T_T"),LOGID);
    }
    return ret;
}

