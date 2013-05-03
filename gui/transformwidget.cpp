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
#include "../tools/transformrequest.h"
#include <QDebug>

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
    plainTextContextMenu = NULL;
    globalContextMenu = NULL;
    infoDialog = NULL;
    sendToMenu = NULL;
    markMenu = NULL;
    copyMenu = NULL;
    importMenu = NULL;
    insertAfterMenu = NULL;
    insertBeforeMenu = NULL;
    replaceMenu = NULL;
    guiHelper = nguiHelper;
    transformFactory = guiHelper->getTransformFactory();
    manager = guiHelper->getNetworkManager();
    logger = guiHelper->getLogger();
    ui->setupUi(this);

    buildSelectionArea();
    ui->wayGroupBox->setVisible(false);
    ui->deleteButton->setEnabled(false);
    ui->infoPushButton->setEnabled(false);
    connect(ui->transfoComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTransformSelected(QString)));

    ui->activityStackedWidget->setCurrentIndex(1);
    ui->transfoComboBox->setFocusPolicy( Qt::StrongFocus );
    ui->transfoComboBox->installEventFilter(guiHelper) ;
    buildContextMenus();
    updateImportExportMenus();
    updateSendToMenu();
    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportMenus()));

    configureViewArea();

    ui->messagesScrollArea->hide();
    firstView = true;
    setAcceptDrops(true);

    connect(transformFactory, SIGNAL(transformsUpdated()),this, SLOT(buildSelectionArea()), Qt::QueuedConnection);
    connect(guiHelper, SIGNAL(filterChanged()), this, SLOT(buildSelectionArea()));
    connect(guiHelper, SIGNAL(tabsUpdated()), this, SLOT(updateSendToMenu()));
    connect(ui->plainTextTab, SIGNAL(invalidText()), this, SLOT(onInvalidText()));
    connect(ui->plainTextTab, SIGNAL(textSelectionChanged()), this, SLOT(onSelectionChanged()));
    connect(ui->backwardPushButton, SIGNAL(clicked()), this, SLOT(onHistoryBackward()));
    connect(ui->forwardPushButton, SIGNAL(clicked()), this, SLOT(onHistoryForward()));

    qDebug() << "Created" << this;
}

TransformWidget::~TransformWidget()
{

    qDebug() << "Destroying:" << this << " " << (currentTransform == NULL ? "Null" : currentTransform->name());

    clearCurrentTransform();

    delete plainTextContextMenu;
    delete globalContextMenu;
    delete infoDialog;
    delete hexTableView;
    // delete dataModel; no need to do that, the TableView should take care of it
    delete sendToMenu;
    delete markMenu;
    delete copyMenu;
    delete importMenu;
    delete insertAfterMenu;
    delete insertBeforeMenu;
    delete replaceMenu;
    delete ui;
    qDebug() << "Destroyed:" << this;
}

void TransformWidget::configureViewArea() {
    hexTableView = new(std::nothrow) ByteTableView(this);
    if (hexTableView == NULL) {
        qFatal("Cannot allocate memory for ByteTableView X{");
        return;
    }
    dataModel = new(std::nothrow) ByteItemModel(hexTableView);
    if (dataModel == NULL) {
        delete hexTableView;
        hexTableView = NULL;
        qFatal("Cannot allocate memory for ByteItemModel X{");
        return;
    }
    connect(dataModel, SIGNAL(updatedFrom(ByteItemModel::UpdateSource)), this, SLOT(updateView(ByteItemModel::UpdateSource)));
    connect(dataModel, SIGNAL(error(QString)), this, SLOT(logError(QString)));
    connect(dataModel, SIGNAL(warning(QString)), this, SLOT(logWarning(QString)));
    hexTableView->setModel(dataModel);

    ((QVBoxLayout *)ui->hexViewTab->layout())->insertWidget(0,hexTableView);
    connect(hexTableView, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    ui->plainTextTab->setModel(dataModel);
    connect(ui->plainTextTab, SIGNAL(sendDropEvent(QDropEvent*)), this, SLOT(dropEvent(QDropEvent*)));
    // creating context menus
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClick(QPoint)));

    nameDialog = new(std::nothrow) QDialog(this);
    if (nameDialog == NULL) {
        qFatal("Cannot allocate memory for nameDialog X{");
        return;
    }

    nameUi.setupUi(nameDialog);

    ui->tabWidget->removeTab(2);
    ui->clearMarkingsPushButton->setEnabled(false);
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

void TransformWidget::buildContextMenus()
{
    sendToMenu = new(std::nothrow) QMenu(tr("Send selection to"));
    if (sendToMenu == NULL) {
        qFatal("Cannot allocate memory for sendToMenu X{");
        return;
    }
    connect(sendToMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSendToTriggered(QAction*)), Qt::UniqueConnection);

    markMenu = new(std::nothrow) QMenu(tr("Mark as"));
    if (markMenu == NULL) {
        qFatal("Cannot allocate memory for markMenu X{");
        return;
    }
    connect(markMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuMarkTriggered(QAction*)), Qt::UniqueConnection);

    copyMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyMenu == NULL) {
        qFatal("Cannot allocate memory for copyMenu X{");
        return;
    }
    connect(copyMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopy(QAction*)), Qt::UniqueConnection);

    importMenu = new(std::nothrow) QMenu(tr("Load from clipboard"));
    if (importMenu == NULL) {
        qFatal("Cannot allocate memory for importMenu X{");
        return;
    }
    connect(importMenu, SIGNAL(triggered(QAction*)), this, SLOT(onImport(QAction*)), Qt::UniqueConnection);

    insertAfterMenu = new(std::nothrow) QMenu(tr("Insert after"));
    if (insertAfterMenu == NULL) {
        qFatal("Cannot allocate memory for insertAfterMenu X{");
        return;
    }
    connect(insertAfterMenu, SIGNAL(triggered(QAction*)), this, SLOT(onInsertAfter(QAction*)), Qt::UniqueConnection);

    replaceMenu = new(std::nothrow) QMenu(tr("Replace selection "));
    if (replaceMenu == NULL) {
        qFatal("Cannot allocate memory for replaceMenu X{");
        return;
    }
    connect(replaceMenu, SIGNAL(triggered(QAction*)), this, SLOT(onReplace(QAction*)), Qt::UniqueConnection);

    insertBeforeMenu = new(std::nothrow) QMenu(tr("Insert before"));
    if (insertBeforeMenu == NULL) {
        qFatal("Cannot allocate memory for insertBeforeMenu X{");
        return;
    }
    connect(insertBeforeMenu, SIGNAL(triggered(QAction*)), this, SLOT(onInsertBefore(QAction*)), Qt::UniqueConnection);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == NULL) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
        return;
    }
    globalContextMenu->addAction(ui->actionSelect_all);
    globalContextMenu->addAction(ui->actionKeep_only_Selected);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->actionNew_byte_array);
    globalContextMenu->addAction(ui->actionImport_From_File);
    globalContextMenu->addMenu(importMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(replaceMenu);
    globalContextMenu->addMenu(insertAfterMenu);
    globalContextMenu->addMenu(insertBeforeMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(markMenu);
    globalContextMenu->addAction(ui->actionClear_marking);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->actionSave_to_file);
    globalContextMenu->addMenu(copyMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->actionDelete_selected_bytes);

}

void TransformWidget::updateSendToMenu()
{
    QAction * action = NULL;
    sendToMenu->clear(); // action created on the fly should be automatically deleted
    sendToActions.clear(); // clearing mapping
    sendToMenu->addAction(ui->actionSend_to_new_tab);
    sendToMenu->addSeparator();

    QList<TransformsGui *> list = guiHelper->getTabs();
    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i)->getName(),sendToMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateSendToMenu X{");
            return;
        }
        sendToActions.insert(action, list.at(i));
        sendToMenu->addAction(action);
    }
}

void TransformWidget::updateMarkMenu()
{
    QAction * action = NULL;
    markMenu->clear(); // action created on the fly should be automatically deleted
    markMenu->addAction(ui->actionNewMarking);
    markMenu->addSeparator();
    QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(48,48);
        pix.fill(i.value());
        action = new(std::nothrow) QAction(QIcon(pix),i.key(), markMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateMarkMenu X{");
            return;
        }
        markMenu->addAction(action);
    }
}

void TransformWidget::updateImportExportMenus()
{
    QAction * action = NULL;
    copyMenu->clear();

    action = new(std::nothrow) QAction(GuiHelper::ACTION_UTF8_STRING, copyMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus UTF8 X{");
        return;
    }
    copyMenu->addAction(action);

    QStringList list = guiHelper->getImportExportFunctions();
    qSort(list);
    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), copyMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus copyMenu X{");
            return;
        }
        copyMenu->addAction(action);
    }

    replaceMenu->clear();
    action = new(std::nothrow) QAction(NEW_BYTE_ACTION, replaceMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu new byte X{");
        return;
    }
    replaceMenu->addAction(action);
    replaceMenu->addSeparator();
    action = new(std::nothrow) QAction(tr("From clipboard as"), replaceMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
        return;
    }
    action->setDisabled(true);
    replaceMenu->addAction(action);
    action = new(std::nothrow) QAction(GuiHelper::ACTION_UTF8_STRING, replaceMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
        return;
    }

    replaceMenu->addAction(action);
    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), replaceMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu user X{");
            return;
        }
        replaceMenu->addAction(action);
    }

    importMenu->clear();
    action = new(std::nothrow) QAction(GuiHelper::ACTION_UTF8_STRING, importMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus importMenu UTF8 X{");
        return;
    }
    importMenu->addAction(action);

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), importMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus importMenu user's X{");
            return;
        }
        importMenu->addAction(action);
    }

    insertAfterMenu->clear();
    action = new(std::nothrow) QAction(NEW_BYTE_ACTION, insertAfterMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu new byte X{");
        return;
    }
    insertAfterMenu->addAction(action);
    insertAfterMenu->addSeparator();
    action = new(std::nothrow) QAction(GuiHelper::ACTION_UTF8_STRING, insertAfterMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu UTF8 X{");
        return;
    }
    insertAfterMenu->addAction(action);

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), insertAfterMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu user's X{");
            return;
        }
        insertAfterMenu->addAction(action);
    }

    insertBeforeMenu->clear();
    action = new(std::nothrow) QAction(NEW_BYTE_ACTION, insertBeforeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertBeforeMenu new byte X{");
        return;
    }
    insertBeforeMenu->addAction(action);
    insertBeforeMenu->addSeparator();
    action = new(std::nothrow) QAction(GuiHelper::ACTION_UTF8_STRING, insertBeforeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertBeforeMenu UTF8 X{");
        return;
    }
    insertBeforeMenu->addAction(action);

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), insertBeforeMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus insertBeforeMenu user's X{");
            return;
        }
        insertBeforeMenu->addAction(action);
    }
}

void TransformWidget::onMenuSendToTriggered(QAction * action)
{
    if (action == ui->actionSend_to_new_tab) {
        guiHelper->sendToNewTab(getSelectedBytes());
    } else {
        if (sendToActions.contains(action)) {
            TransformsGui * tg = sendToActions.value(action);
            tg->setData(getSelectedBytes());
            tg->bringFront();
        } else {
            logger->logError(tr("Tab not found for sending"));
        }
    }
}

void TransformWidget::onMenuMarkTriggered(QAction * action)
{
    QString name;
    if (action == ui->actionNewMarking) {
        if (ui->tabWidget->currentWidget() == ui->plainTextTab) {
            logger->logError(tr("The Mark function should not be called on the text view"));
            return;
        } else if (!hexTableView->hasSelection()) {
            return;
        }

        QColor choosenColor = QColorDialog::getColor(Qt::yellow, this);

        QPixmap pix(20,20);
        pix.fill(choosenColor);
        nameUi.nameLabel->setPixmap(pix);
        int ret = nameDialog->exec();
        if (ret == QDialog::Accepted) {
            name = nameUi.nameLineEdit->text();
            if (!name.isEmpty())
                guiHelper->addNewMarkingColor(name,choosenColor);
            hexTableView->markSelected(choosenColor, name);
            ui->clearMarkingsPushButton->setEnabled(dataModel->hasMarking());
        }
    } else {
        name = action->text();
        QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
        if (colors.contains(name)) {
            hexTableView->markSelected(colors.value(name), name);
            ui->clearMarkingsPushButton->setEnabled(dataModel->hasMarking());
        } else {
            logger->logError(tr("Unknown marking color"));
        }
    }
}

void TransformWidget::clearCurrentTransform()
{
    if (currentTransform != NULL) {
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

void TransformWidget::onCopy(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (action->text() == GuiHelper::ACTION_UTF8_STRING) {
        clipboard->setText(QString::fromUtf8(getSelectedBytes()));
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != NULL) {
            ta->setWay(TransformAbstract::INBOUND);
            clipboard->setText(ta->transform(getSelectedBytes()));
        }
    }
}

void TransformWidget::onImport(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this);
        if (dialog == NULL) {
            qFatal("Cannot allocate memory for action NewByteDialog X{");
            return;
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            dataModel->setRawData(QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;

    } else if (action->text() == GuiHelper::ACTION_UTF8_STRING) {
        dataModel->setRawData(input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != NULL) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->setRawData(ta->transform(input.toUtf8()));
        }
    }
}

void TransformWidget::onReplace(QAction *action)
{
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this,true);
        if (dialog == NULL) {
            qFatal("Cannot allocate memory for onReplace NewByteDialog X{");
            return;
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            hexTableView->replaceSelectedBytes(dialog->getChar());
        }
        delete dialog;
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        QString input = clipboard->text();

        if (action->text() == GuiHelper::ACTION_UTF8_STRING) {
            hexTableView->replaceSelectedBytes(input.toUtf8());
        } else {
            TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
            if (ta != NULL) {
                ta->setWay(TransformAbstract::OUTBOUND);
                hexTableView->replaceSelectedBytes(ta->transform(input.toUtf8()));
            }
        }
    }
}

void TransformWidget::onInsertAfter(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    int pos = hexTableView->getHigherSelected() + 1;
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this);
        if (dialog == NULL) {
            qFatal("Cannot allocate memory for onInsertAfter NewByteDialog X{");
            return;
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            dataModel->insert(pos, QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;
    } else if (action->text() == GuiHelper::ACTION_UTF8_STRING) {
        dataModel->insert(pos,input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != NULL) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->insert(pos,ta->transform(input.toUtf8()));
        }
    }
}

void TransformWidget::onInsertBefore(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    int pos = hexTableView->getLowerSelected();
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this);
        if (dialog == NULL) {
            qFatal("Cannot allocate memory for onInsertBefore NewByteDialog X{");
            return;
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            dataModel->insert(pos, QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;
    } else if (action->text() == GuiHelper::ACTION_UTF8_STRING) {
        dataModel->insert(pos,input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != NULL) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->insert(pos,ta->transform(input.toUtf8()));
        }
    }
}

void TransformWidget::onHistoryBackward()
{
    dataModel->historyBackward();
}

void TransformWidget::onHistoryForward()
{
    dataModel->historyForward();
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

        QWidget *confGui = currentTransform->getGui(this);
        if (confGui != NULL) {
            ui->settingsTab->layout()->addWidget(confGui);
            ui->tabWidget->insertTab(2,ui->settingsTab, tr("Settings"));
        }
        configureDirectionBox();
        connect(currentTransform,SIGNAL(confUpdated()),this,SLOT(updatingFromTransform()));
        connect(currentTransform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString)));
        connect(currentTransform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString)));

        ui->deleteButton->setEnabled(true);
        ui->infoPushButton->setEnabled(true);
        emit transformChanged();
        refreshOutput();
    }
}

void TransformWidget::input(QByteArray text) {
    dataModel->setRawData(text);
    if (firstView) {
        if (dataModel->isReadableText()) {
            ui->tabWidget->setCurrentIndex(0);
        }
        else {
            ui->tabWidget->setCurrentIndex(1);
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
        inputData = dataModel->getRawData();
        TransformAbstract * ta = transformFactory->loadTransformFromConf(currentTransform->getConfiguration());
        if (ta != NULL) {
            TransformRequest *tr = new TransformRequest(
                        ta,
                        dataModel->getRawData(),
                        (quintptr) this);

            connect(tr,SIGNAL(finishedProcessing(QByteArray,Messages)), this, SLOT(processingFinished(QByteArray,Messages)));
            guiHelper->processTransform(tr);
        }
    }
}

void TransformWidget::processingFinished(QByteArray output, Messages messages)
{
    clearMessages();
    outputData = output;

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

    emit updated();
    ui->activityStackedWidget->setCurrentIndex(1);
}

void TransformWidget::on_actionNew_byte_array_triggered()
{
    NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this);
    if (dialog == NULL) {
        qFatal("Cannot allocate memory for on_actionNew_byte_array_triggered NewByteDialog X{");
        return;
    }
    dialog->setModal(true);
    int ret = dialog->exec();
    if (ret == QDialog::Accepted) {
        dataModel->setRawData(QByteArray(dialog->byteCount(),dialog->getChar()));
    }
    delete dialog;
}

void TransformWidget::on_actionImport_From_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"));
    if (!fileName.isEmpty())
        dataModel->fromLocalFile(fileName);
}


void TransformWidget::on_actionSave_to_file_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"));
    if (!fileName.isEmpty()) {

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QString mess = tr("Failed to open the file for writing:\n %1").arg(file.errorString());
            logger->logError(mess);
            QMessageBox::critical(this,tr("Error"), mess,QMessageBox::Ok);
            return;
        }
        file.write(dataModel->getRawData());
        file.close();
    }
}

void TransformWidget::on_actionDelete_selected_bytes_triggered()
{
    if (ui->tabWidget->currentWidget() == ui->plainTextTab) {
        QString mess = tr("[TransformWidget] This function should never be called on the Text view. T_T");
        qWarning() << mess;
        QMessageBox::critical(this,tr("Error"),mess,QMessageBox::Ok);
    } else if (ui->tabWidget->currentWidget() == ui->hexViewTab) {
        hexTableView->deleteSelectedBytes();
    }
}

QByteArray TransformWidget::getSelectedBytes() {
    QByteArray ret;
    if (ui->tabWidget->currentWidget() == ui->plainTextTab) {
        ret  = ui->plainTextTab->getSelection();
    } else if (ui->tabWidget->currentWidget() == ui->hexViewTab) {
        ret = hexTableView->getSelectedBytes();
    }
    return ret;
}

void TransformWidget::onRightClick(QPoint pos)
{
    if (ui->tabWidget->currentWidget() == ui->plainTextTab) { // on text view
        ui->actionDelete_selected_bytes->setVisible(false);
        insertAfterMenu->setEnabled(false);
        insertBeforeMenu->setEnabled(false);
        markMenu->setEnabled(false);
        replaceMenu->setEnabled(false);
        ui->actionClear_marking->setVisible(false);
        if (ui->plainTextTab->hasSelection()) { // if any selection
            copyMenu->setEnabled(true);
            ui->actionKeep_only_Selected->setEnabled(true);
            sendToMenu->setEnabled(true);
        } else {
            copyMenu->setEnabled(false);
            ui->actionKeep_only_Selected->setEnabled(false);
            sendToMenu->setEnabled(false);
        }
    } else { // if on hex view
        updateMarkMenu();
        ui->actionDelete_selected_bytes->setVisible(true);
        insertAfterMenu->setEnabled(true);
        insertBeforeMenu->setEnabled(true);
        markMenu->setEnabled(true);

        ui->actionClear_marking->setVisible(true);
        if (hexTableView->getSelectedBytes().isEmpty()) { // if any selection
            copyMenu->setEnabled(false);
            ui->actionDelete_selected_bytes->setEnabled(false);
            ui->actionKeep_only_Selected->setEnabled(false);
            sendToMenu->setEnabled(false);
            markMenu->setEnabled(false);
            ui->actionClear_marking->setEnabled(false);
            replaceMenu->setEnabled(false);
        } else {
            copyMenu->setEnabled(true);
            ui->actionDelete_selected_bytes->setEnabled(true);
            ui->actionKeep_only_Selected->setEnabled(true);
            sendToMenu->setEnabled(true);
            markMenu->setEnabled(true);
            replaceMenu->setEnabled(true);
            ui->actionClear_marking->setEnabled(dataModel->hasMarking());
        }
    }
    if (dataModel->size() == 0) {
        ui->actionSelect_all->setEnabled(false);
    } else {
        ui->actionSelect_all->setEnabled(true);
    }
    globalContextMenu->exec(this->mapToGlobal(pos));
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
    TransformWidget* src = (TransformWidget* ) sender();
    clearMessages();

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

void TransformWidget::clearMessages() {
    ui->messagesDisplay->clear();
    ui->messagesScrollArea->hide();
}

void TransformWidget::reset()
{
    if (currentTransform != NULL) {
        clearCurrentTransform();

        ui->transfoComboBox->blockSignals(true);
        ui->transfoComboBox->setCurrentIndex(0);
        ui->transfoComboBox->blockSignals(false);
        ui->wayGroupBox->setVisible(false);
        ui->tabWidget->removeTab(2);
        ui->descriptionLabel->clear();
    }
    ui->plainTextTab->reset();
    ui->deleteButton->setEnabled(false);
    ui->infoPushButton->setEnabled(false);
    ui->tabWidget->setCurrentIndex(0);
}

void TransformWidget::setDownload(QUrl url)
{
    if (manager != NULL) {
        DownloadManager * downloadManager = new(std::nothrow) DownloadManager(url, manager, dataModel);
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

void TransformWidget::downloadFinished(DownloadManager * downloadManager)
{
    ui->activityStackedWidget->setCurrentIndex(1);
    delete downloadManager;
}

void TransformWidget::updateView(ByteItemModel::UpdateSource)
{
    updateStats();
    refreshOutput();
}

void TransformWidget::updateStats()
{
    // updating various stats
    QString ret("Size: ");
    int size = dataModel->size();

    // Updating Hex info
    ret.append(QString::number(size)).append(" bytes");
    if (size >= 1000 && size < 1000000)
        ret.append(QString(" / %1").arg((double)size/(double)1000,0,'f',2)).append(" KiB");
    else if (size >= 1000000 && size < 1000000000)
        ret.append(QString(" / %1").arg((double)size/(double)1000000,0,'f',2)).append(" MiB");

    int scount = hexTableView->getSelectedBytesCount();
    if ( scount != 0) {
        ret.append(tr(" (%1 selected)").arg(scount));
    }

    ui->sizeHexLabel->setText(ret);
}

void TransformWidget::onSelectionChanged()
{
    // notifying the upper layer
    guiHelper->sendNewSelection(getSelectedBytes());
    updateStats();
}

void TransformWidget::onInvalidText()
{
    ui->tabWidget->setCurrentWidget(ui->hexViewTab);
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

            dataModel->fromLocalFile(filename);
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
                dataModel->setRawData(event->mimeData()->text().toUtf8());
            }
        } else {
            dataModel->setRawData(event->mimeData()->text().toUtf8());

        }
        event->acceptProposedAction();
    } else {
        logger->logWarning(tr("Don't know how to handle this Drop action"));
    }
}

void TransformWidget::onTransformConfError(const QString mess, const QString source)
{
    emit confErrors(mess, source);
    emit error(mess,source);
}

void TransformWidget::on_deleteButton_clicked()
{
    emit deletionRequest(this);
}

void TransformWidget::addNullBytes(char byteSample, int pos, int count)
{
    dataModel->insert(pos, QByteArray(count,byteSample));
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

void TransformWidget::on_actionSelect_all_triggered()
{
    if (ui->tabWidget->currentWidget() == ui->plainTextTab) {
        ui->plainTextTab->selectAll();
    } else {
        hexTableView->selectAllBytes();
    }
}

void TransformWidget::on_actionKeep_only_Selected_triggered()
{
    input(getSelectedBytes());
}

void TransformWidget::on_clearMarkingsPushButton_clicked()
{
    dataModel->clearAllMarkings();
    ui->clearMarkingsPushButton->setEnabled(false);
}

void TransformWidget::on_actionClear_marking_triggered()
{
    if (ui->tabWidget->currentWidget() == ui->plainTextTab) {
        logger->logError(tr("This function should not be called on the text view"));
        return;
    } else if (!hexTableView->hasSelection()) {
        return;
    }

    hexTableView->clearMarkOnSelected();
    ui->clearMarkingsPushButton->setEnabled(dataModel->hasMarking());
}

void TransformWidget::on_clearDataPushButton_clicked()
{
    dataModel->clear();
}
