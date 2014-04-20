/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QClipboard>
#include <QFileDialog>
#include <QColorDialog>
#include <QMenu>
#include <QMessageBox>
#include <QAction>
#include <QDebug>
#include <QTextEncoder>
#include "../textinputdialog.h"
#include "../newbytedialog.h"
#include "hexview.h"
#include "ui_hexview.h"
#include "../tabs/tababstract.h"
#include "../sources/bytesourceabstract.h"
#include "byteitemmodel.h"
#include "bytetableview.h"
#include "../loggerwidget.h"
#include "../guihelper.h"
#include "../crossplatform.h"
#include <transformabstract.h>

const QString HexView::NEW_BYTE_ACTION = "New Byte(s)";
const QString HexView::BYTES_LE_ACTION = "Little endian";
const QString HexView::BYTES_BE_ACTION = "Big endian";
const QString HexView::SIZE_OCTAL_ACTION = "Octal";
const QString HexView::SIZE_DECIMAL_ACTION = "Decimal";
const QString HexView::SIZE_HEXADECIMAL_ACTION = "Hexadecimal";
const QString HexView::OFFSET_LE_ACTION = "Absolute LE";
const QString HexView::OFFSET_BE_ACTION = "Absolute BE";
const QString HexView::OFFSET_REL_LE_ACTION = "Relative LE";
const QString HexView::OFFSET_REL_BE_ACTION = "Relative BE";

HexView::HexView(ByteSourceAbstract *nbyteSource,GuiHelper *nguiHelper, QWidget *parent) :
    SingleViewAbstract(nbyteSource, nguiHelper, parent)
{
    ui = new(std::nothrow) Ui::HexView;
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::HexView X{");
    }

    globalContextMenu = NULL;
    sendToMenu = NULL;
    markMenu = NULL;
    copyMenu = NULL;
    loadMenu = NULL;
    copySelectedSizeMenu = NULL;
    insertAfterMenu = NULL;
    insertBeforeMenu = NULL;
    replaceMenu = NULL;
    selectFromSizeMenu = NULL;
    gotoFromOffsetMenu = NULL;
    copyCurrentOffsetMenu = NULL;
    selectedSize = 0;
    startOffset = 0;
    ui->setupUi(this);
    hexTableView = new(std::nothrow) ByteTableView(this);
    if (hexTableView == NULL) {
        qFatal("Cannot allocate memory for ByteTableView X{");
    }
    connect(hexTableView, SIGNAL(error(QString,QString)), logger, SLOT(logError(QString,QString)));
    connect(hexTableView, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));


    dataModel = new(std::nothrow) ByteItemModel(byteSource,hexTableView);
    if (dataModel == NULL) {
        delete hexTableView;
        hexTableView = NULL;
        qFatal("Cannot allocate memory for ByteItemModel X{");
    }

    connect(byteSource, SIGNAL(updated(quintptr)), this, SLOT(updateStats()));

    connect(dataModel, SIGNAL(error(QString)), logger, SLOT(logError(QString)));
    connect(dataModel, SIGNAL(warning(QString)), logger, SLOT(logWarning(QString)));
    hexTableView->setModel(dataModel);

    ui->mainLayout->insertWidget(0,hexTableView);
    connect(hexTableView, SIGNAL(newSelection()), this, SLOT(onSelectionChanged()));

    // creating context menus
    buildContextMenus();
    updateImportExportMenus();
    updateSendToMenu();
    updateMarkMenu();
    connect(guiHelper, SIGNAL(tabsUpdated()), this, SLOT(updateSendToMenu()));
    connect(guiHelper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportMenus()));
    connect(guiHelper, SIGNAL(markingsUpdated()), this, SLOT(updateMarkMenu()));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClick(QPoint)));

    updateStats();
}

HexView::~HexView()
{
    delete hexTableView;
    dataModel = NULL; //no need to delete it, the TableView should take care of it
    byteSource = NULL;
    delete globalContextMenu;
    delete sendToMenu;
    delete markMenu;
    delete copyMenu;
    delete loadMenu;
    delete copySelectedSizeMenu;
    delete insertAfterMenu;
    delete insertBeforeMenu;
    delete replaceMenu;
    delete selectFromSizeMenu;
    delete copyCurrentOffsetMenu;
    delete ui;
    logger = NULL;
    guiHelper = NULL;
}

void HexView::addCustomMenuActions(QAction *action)
{
    QAction * prev = globalContextMenu->actionAt(QPoint(0,0));
    globalContextMenu->insertAction(prev,action);
    globalContextMenu->insertSeparator(prev);
}

ByteTableView *HexView::getHexTableView()
{
    return hexTableView;
}

void HexView::updateStats()
{
    // updating various stats
    QString ret;
    quint64 size = byteSource->size();

    if (size != ULONG_LONG_MAX) {
        ret.append("Size: ");
        // Updating Hex info
        ret.append(QString::number(size)).append("|x").append(QString::number(size,16)).append(" bytes");
        if (size >= 1000 && size < 1000000)
            ret.append(QString(" (%1").arg((double)size/(double)1000,0,'f',2)).append(" KiB)");
        else if (size >= 1000000 && size < 1000000000)
            ret.append(QString(" (%1").arg((double)size/(double)1000000,0,'f',2)).append(" MiB)");
    }


    int scount = hexTableView->getSelectedBytesCount();
    //hexTableView->verticalHeader()->adjustSize();
    if ( scount != 0) {
        ret.append(tr(" [ %1|x%2 selected ]").arg(scount).arg(QString::number(scount,16)));
    }

    ui->statsLabel->setText(ret);

    //Updating offset

    ret = "Offset: ";
    int offset =  hexTableView->getCurrentPos();
    if (offset < 0) {
        ret.append("NA");
    } else {
        quint64 finaloffset = offset + byteSource->startingRealOffset();
        ret.append(QString::number(finaloffset));
        ret.append(" | x");
        ret.append(QString::number(finaloffset,16));
        ret.append(" | o");
        ret.append(QString::number(finaloffset,8));
    }
    ui->offsetLabel->setText(ret);
}

void HexView::onRightClick(QPoint pos)
{
    if (hexTableView->getSelectedBytes().isEmpty()) { // No selection
        copyMenu->setEnabled(false);
        ui->deleteSelectionAction->setEnabled(false);
        ui->keepOnlySelectionAction->setEnabled(false);
        sendToMenu->setEnabled(false);
        markMenu->setEnabled(false);
        ui->clearMarkingsAction->setEnabled(false);
        replaceMenu->setEnabled(false);
        selectFromSizeMenu->setEnabled(false);
        gotoFromOffsetMenu->setEnabled(false);
        copySelectedSizeMenu->setEnabled(false);
        ui->saveSelectedToFileAction->setEnabled(false);
    } else { // selected bytes
        copyMenu->setEnabled(true);
        ui->deleteSelectionAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
        ui->keepOnlySelectionAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
        sendToMenu->setEnabled(true);
        markMenu->setEnabled(true);
        replaceMenu->setEnabled(!byteSource->isReadonly());
        selectFromSizeMenu->setEnabled(true);
        gotoFromOffsetMenu->setEnabled(true);
        ui->clearMarkingsAction->setEnabled(byteSource->hasMarking());
        copySelectedSizeMenu->setEnabled(true);
        ui->saveSelectedToFileAction->setEnabled(true);
    }
    ui->newByteArrayAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESET) && !byteSource->isReadonly());
    loadMenu->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESET) && !byteSource->isReadonly());

    if (byteSource->size() <= 0) {
        ui->selectAllAction->setEnabled(false);
        ui->saveToFileAction->setEnabled(false);
        saveToFileMenu->setEnabled(false);
        copyCurrentOffsetMenu->setEnabled(false);
        insertAfterMenu->setEnabled(false);
        insertBeforeMenu->setEnabled(false);
    } else {
        ui->selectAllAction->setEnabled(true);
        ui->saveToFileAction->setEnabled(true);
        saveToFileMenu->setEnabled(true);
        copyCurrentOffsetMenu->setEnabled(true);
        insertAfterMenu->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
        insertBeforeMenu->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_RESIZE) && !byteSource->isReadonly());
    }
    ui->importFileAction->setEnabled(byteSource->hasCapability(ByteSourceAbstract::CAP_LOADFILE));

    globalContextMenu->exec(this->mapToGlobal(pos));
}

void HexView::updateMarkMenu()
{
    markMenu->clear(); // action created on the fly should be automatically deleted
    markMenu->addAction(ui->newMarkingAction);
    markMenu->addSeparator();
    QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(48,48);
        pix.fill(i.value());
        QAction *  action = new(std::nothrow) QAction(QIcon(pix),i.key(), markMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateMarkMenu X{");
            return;
        }
        markMenu->addAction(action);
    }
}

void HexView::updateImportExportMenus()
{
    QAction * action = NULL;

    guiHelper->updateCopyContextMenu(copyMenu);
    guiHelper->updateLoadContextMenu(loadMenu);

    QStringList list = guiHelper->getImportExportFunctions();

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
    action = new(std::nothrow) QAction(GuiHelper::UTF8_STRING_ACTION, replaceMenu);
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

    insertAfterMenu->clear();
    action = new(std::nothrow) QAction(NEW_BYTE_ACTION, insertAfterMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus insertAfterMenu new byte X{");
        return;
    }
    insertAfterMenu->addAction(action);
    insertAfterMenu->addSeparator();
    action = new(std::nothrow) QAction(GuiHelper::UTF8_STRING_ACTION, insertAfterMenu);
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
    action = new(std::nothrow) QAction(GuiHelper::UTF8_STRING_ACTION, insertBeforeMenu);
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

void HexView::updateSendToMenu()
{
    sendToMenu->clear(); // action created on the fly should be automatically deleted
    sendToTabMapping.clear(); // clearing mapping
    sendToMenu->addAction(ui->sendToNewTabAction);
    sendToMenu->addSeparator();

    QList<TabAbstract *> list = guiHelper->getTabs();
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i)->canReceiveData()) {
            QAction * action = new(std::nothrow) QAction(list.at(i)->getName(),sendToMenu);
            if (action == NULL) {
                qFatal("Cannot allocate memory for action updateSendToMenu X{");
                return;
            }
            sendToTabMapping.insert(action, list.at(i));
            sendToMenu->addAction(action);
        }
    }
}

void HexView::onCopy(QAction *action)
{
    guiHelper->copyAction(action->text(), hexTableView->getSelectedBytes());
}

void HexView::onLoad(QAction *action)
{
    guiHelper->loadAction(action->text(), byteSource);
}

void HexView::onReplace(QAction *action)
{
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this,true);
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

        if (action->text() == GuiHelper::UTF8_STRING_ACTION) {
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

void HexView::onInsertAfter(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    int pos = hexTableView->getHigherSelected() + 1;
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this);
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
    } else if (action->text() == GuiHelper::UTF8_STRING_ACTION) {
        dataModel->insert(pos,input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != NULL) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->insert(pos,ta->transform(input.toUtf8()));
        }
    }
}

void HexView::onInsertBefore(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    int pos = hexTableView->getLowerSelected();
    if (action->text() == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this);
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
    } else if (action->text() == GuiHelper::UTF8_STRING_ACTION) {
        dataModel->insert(pos,input.toUtf8());
    } else {
        TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
        if (ta != NULL) {
            ta->setWay(TransformAbstract::OUTBOUND);
            dataModel->insert(pos,ta->transform(input.toUtf8()));
        }
    }
}

void HexView::onSendToTab(QAction *action)
{
    if (action == ui->sendToNewTabAction) {
        guiHelper->sendToNewTab(hexTableView->getSelectedBytes());
    } else {
        if (sendToTabMapping.contains(action)) {
            TabAbstract * tg = sendToTabMapping.value(action);
            tg->setData(hexTableView->getSelectedBytes());
            tg->bringFront();
        } else {
            logger->logError(tr("Tab not found for sending"));
        }
    }
}

void HexView::onMarkMenu(QAction *action)
{
    QString name;
    if (action == ui->newMarkingAction) {
        if (hexTableView->hasSelection()) {
            QColor choosenColor = QColorDialog::getColor(Qt::yellow, this);

            QPixmap pix(20,20);
            pix.fill(choosenColor);
            TextInputDialog *nameDialog = new(std::nothrow) TextInputDialog(this);
            if (nameDialog == NULL) {
                qFatal("Cannot allocate memory for textInputDialog X{");
            }
            nameDialog->setPixLabel(pix);
            int ret = nameDialog->exec();
            if (ret == QDialog::Accepted) {
                name = nameDialog->getInputText();
                if (!name.isEmpty())
                    guiHelper->addNewMarkingColor(name,choosenColor);
                hexTableView->markSelected(choosenColor, name);
            }
        }

    } else {
        name = action->text();
        QHash<QString, QColor> colors = guiHelper->getMarkingsColor();
        if (colors.contains(name)) {
            hexTableView->markSelected(colors.value(name), name);
        } else {
            qCritical("Unknown marking color T_T");
        }
    }
}

void HexView::onSelectFromSizeMenu(QAction *action)
{
    quint64 val = normalizeSelectedInt(action->text() == BYTES_BE_ACTION);
    if (val == 0) {
        return;
    }

    qint64 inter = byteSource->size() - hexTableView->getHigherSelected();
    if (val >= (quint64)(inter < 0 ? 0 : inter) ) {
        QString mess = tr("This size value would select out-of-bound (maybe the selected value is a signed int)");
        logger->logError(mess);
        QMessageBox::warning(this, tr("Value too large"), mess, QMessageBox::Ok);
    } else {
        qint64 pos = hexTableView->getHigherSelected() + 1;
        hexTableView->selectBytes(pos, val);
    }
}

void HexView::onGotoFromOffsetMenu(QAction *action)
{
    bool absolute = true;
    quint64 val = 0;
    if (action->text() == OFFSET_BE_ACTION || action->text() == OFFSET_LE_ACTION) {
        val = normalizeSelectedInt(action->text() == OFFSET_BE_ACTION);
        if (val == 0) {
            return;
        }
    } else {
        val = normalizeSelectedInt(action->text() == OFFSET_REL_BE_ACTION);
        if (val == 0) {
            return;
        }
        qint64 inter = byteSource->size() - hexTableView->getHigherSelected();
        if (val >= (quint64)(inter < 0 ? 0 : inter)) {
            QString mess = tr("This offset value would go out-of-bound (maybe the selected value is a signed int)");
            logger->logError(mess);
            QMessageBox::warning(this, tr("Value too large"), mess, QMessageBox::Ok);
            return;
        }
        absolute = false;
    }

    if (!hexTableView->goTo(val, absolute,false)) {
        QMessageBox::warning(this, tr("Offset error"), tr("Offset value error"), QMessageBox::Ok);
    }


}

void HexView::onCopySelectedSize(QAction *action)
{
    selectedSize = hexTableView->getSelectedBytesCount();
    QString text;
    if (action->text() == SIZE_OCTAL_ACTION) {
        text = QString::number(selectedSize,8);
    } else if (action->text() == SIZE_DECIMAL_ACTION) {
        text = QString::number(selectedSize,10);
    } else if (action->text() == SIZE_HEXADECIMAL_ACTION) {
        text = QString::number(selectedSize,16);
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void HexView::onCopyCurrentOffset(QAction *action)
{
    if (hexTableView->getCurrentPos() >= 0) {
        quint64 currenOffset = byteSource->getRealOffset(hexTableView->getCurrentPos());
        QString text;
        if (action->text() == SIZE_OCTAL_ACTION) {
            text = QString::number(currenOffset,8);
        } else if (action->text() == SIZE_DECIMAL_ACTION) {
            text = QString::number(currenOffset,10);
        } else if (action->text() == SIZE_HEXADECIMAL_ACTION) {
            text = QString::number(currenOffset,16);
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(text);
    }
}

quint64 HexView::normalizeSelectedInt(bool bigEndian)
{
    QByteArray bytesdata = hexTableView->getSelectedBytes();
    quint64 size = bytesdata.size(); // converting int to quint64, no drama here
    if (size > 8) {
        QString mess = tr("Too many bytes selected for an uint 64 (max 8 bytes)");
        logger->logError(mess);
        QMessageBox::warning(this, tr("Too many bytes"), mess, QMessageBox::Ok);
        return 0;
    }


    bool reverseByteOrdering = false;

#ifdef Q_LITTLE_ENDIAN
    bool currentSystemLittleEndian = true;
#else
    bool currentSystemLittleEndian = false;
#endif

    if (currentSystemLittleEndian == bigEndian) {
        reverseByteOrdering = true;
    }
    if (size < sizeof(quint64)) {
        if (reverseByteOrdering)
            bytesdata.prepend(QByteArray(sizeof(quint64) - bytesdata.size(),'\x00'));
        else
            bytesdata.append(QByteArray(sizeof(quint64) - bytesdata.size(),'\x00'));
    }

    if (reverseByteOrdering) {
        QByteArray temp2;
        for (int i = bytesdata.size() - 1; i >= 0; i--) {
            temp2.append(bytesdata.at(i));
        }
        bytesdata = temp2;
    }
    quint64 val = 0;

    memcpy(&val,bytesdata.constData(),sizeof(quint64));

    return val;
}

void HexView::onClearSelectionMarkings()
{
    if (hexTableView->hasSelection()) {
        hexTableView->clearMarkOnSelected();
    }
}

void HexView::onClearAllMArkings()
{
    byteSource->clearAllMarkings();
}

bool HexView::goTo(quint64 offset, bool absolute, bool negative, bool select)
{
    return hexTableView->goTo(offset,absolute, negative, select);
}

void HexView::search(QByteArray item, QBitArray mask)
{
    hexTableView->search(item, mask);
}

void HexView::searchAgain()
{
    hexTableView->searchAgain();
}

void HexView::onLoadFile()
{
    emit askForFileLoad();
}

void HexView::onSaveToFile(QAction *action)
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"));
    if (!fileName.isEmpty()) {
        if (action == ui->saveSelectedToFileAction) {
            qDebug() << "save selection";
            byteSource->saveToFile(fileName,byteSource->getRealOffset(hexTableView->getLowerSelected()), byteSource->getRealOffset(hexTableView->getHigherSelected()));
        }
        else
            byteSource->saveToFile(fileName);
    }
}

void HexView::onSelectAll()
{
    hexTableView->selectAllBytes();
}

void HexView::onKeepOnlySelection()
{
    byteSource->setData(hexTableView->getSelectedBytes());
}

void HexView::onNewByteArray()
{
    NewByteDialog *dialog = new(std::nothrow) NewByteDialog(guiHelper, this);
    if (dialog == NULL) {
        qFatal("Cannot allocate memory for onNewByteArray NewByteDialog X{");
        return;
    }
    dialog->setModal(true);
    int ret = dialog->exec();
    if (ret == QDialog::Accepted) {
        byteSource->setData(QByteArray(dialog->byteCount(),dialog->getChar()));
    }
    delete dialog;
}

void HexView::onDeleteSelection()
{
    hexTableView->deleteSelectedBytes();
}

void HexView::onSelectionChanged()
{
    guiHelper->sendNewSelection(hexTableView->getSelectedBytes());
    updateStats();
}

void HexView::buildContextMenus()
{
    sendToMenu = new(std::nothrow) QMenu(tr("Send selection to"));
    if (sendToMenu == NULL) {
        qFatal("Cannot allocate memory for sendToMenu X{");
        return;
    }
    connect(sendToMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSendToTab(QAction*)), Qt::UniqueConnection);

    markMenu = new(std::nothrow) QMenu(tr("Mark as"));
    if (markMenu == NULL) {
        qFatal("Cannot allocate memory for markMenu X{");
        return;
    }
    connect(markMenu, SIGNAL(triggered(QAction*)), this, SLOT(onMarkMenu(QAction*)), Qt::UniqueConnection);

    copyMenu = new(std::nothrow) QMenu(tr("Copy as"));
    if (copyMenu == NULL) {
        qFatal("Cannot allocate memory for copyMenu X{");
        return;
    }
    connect(copyMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopy(QAction*)), Qt::UniqueConnection);

    loadMenu = new(std::nothrow) QMenu(tr("Load from clipboard"));
    if (loadMenu == NULL) {
        qFatal("Cannot allocate memory for importMenu X{");
        return;
    }
    connect(loadMenu, SIGNAL(triggered(QAction*)), this, SLOT(onLoad(QAction*)), Qt::UniqueConnection);

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

    selectFromSizeMenu = new(std::nothrow) QMenu(tr("Select from size"));
    if (selectFromSizeMenu == NULL) {
        qFatal("Cannot allocate memory for selectFromSizeMenu X{");
        return;
    }
    connect(selectFromSizeMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSelectFromSizeMenu(QAction*)), Qt::UniqueConnection);

    QAction * action = NULL;
    action = new(std::nothrow) QAction(BYTES_LE_ACTION, selectFromSizeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SELECT_LE_ACTION X{");
        return;
    }
    selectFromSizeMenu->addAction(action);

    action = new(std::nothrow) QAction(BYTES_BE_ACTION, selectFromSizeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SELECT_BE_ACTION X{");
        return;
    }
    selectFromSizeMenu->addAction(action);

    gotoFromOffsetMenu = new(std::nothrow) QMenu(tr("Goto selected offset"));
    if (gotoFromOffsetMenu == NULL) {
        qFatal("Cannot allocate memory for gotoFromOffsetMenu X{");
        return;
    }
    connect(gotoFromOffsetMenu, SIGNAL(triggered(QAction*)), this, SLOT(onGotoFromOffsetMenu(QAction*)), Qt::UniqueConnection);

    action = NULL;
    action = new(std::nothrow) QAction(OFFSET_LE_ACTION, gotoFromOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SELECT_LE_ACTION X{");
        return;
    }
    gotoFromOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(OFFSET_BE_ACTION, gotoFromOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SELECT_LE_ACTION X{");
        return;
    }
    gotoFromOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(OFFSET_REL_LE_ACTION, gotoFromOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SELECT_BE_ACTION X{");
        return;
    }
    gotoFromOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(OFFSET_REL_BE_ACTION, gotoFromOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SELECT_BE_ACTION X{");
        return;
    }
    gotoFromOffsetMenu->addAction(action);

    copySelectedSizeMenu = new(std::nothrow) QMenu(tr("Copy selected size"));
    if (copySelectedSizeMenu == NULL) {
        qFatal("Cannot allocate memory for copySelectedSizeMenu X{");
        return;
    }
    connect(copySelectedSizeMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopySelectedSize(QAction*)), Qt::UniqueConnection);

    action = new(std::nothrow) QAction(SIZE_OCTAL_ACTION, copySelectedSizeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SIZE_OCTAL_ACTION X{");
        return;
    }
    copySelectedSizeMenu->addAction(action);

    action = new(std::nothrow) QAction(SIZE_DECIMAL_ACTION, copySelectedSizeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SIZE_DECIMAL_ACTION X{");
        return;
    }
    copySelectedSizeMenu->addAction(action);

    action = new(std::nothrow) QAction(SIZE_HEXADECIMAL_ACTION, copySelectedSizeMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SIZE_HEXADECIMAL_ACTION X{");
        return;
    }
    copySelectedSizeMenu->addAction(action);

    copyCurrentOffsetMenu = new(std::nothrow) QMenu(tr("Copy offset value"));
    if (copyCurrentOffsetMenu == NULL) {
        qFatal("Cannot allocate memory for copyCurrentOffsetMenu X{");
        return;
    }
    connect(copyCurrentOffsetMenu, SIGNAL(triggered(QAction*)), this, SLOT(onCopyCurrentOffset(QAction*)), Qt::UniqueConnection);

    action = new(std::nothrow) QAction(SIZE_OCTAL_ACTION, copyCurrentOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SIZE_OCTAL_ACTION X{");
        return;
    }
    copyCurrentOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(SIZE_DECIMAL_ACTION, copyCurrentOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SIZE_DECIMAL_ACTION X{");
        return;
    }
    copyCurrentOffsetMenu->addAction(action);

    action = new(std::nothrow) QAction(SIZE_HEXADECIMAL_ACTION, copyCurrentOffsetMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for SIZE_HEXADECIMAL_ACTION X{");
        return;
    }
    copyCurrentOffsetMenu->addAction(action);

    saveToFileMenu = new(std::nothrow) QMenu(tr("Save to file"));
    if (saveToFileMenu == NULL) {
        qFatal("Cannot allocate memory for saveToFile X{");
        return;
    }
    connect(saveToFileMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSaveToFile(QAction*)), Qt::UniqueConnection);
    saveToFileMenu->addAction(ui->saveToFileAction);
    saveToFileMenu->addAction(ui->saveSelectedToFileAction);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == NULL) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
        return;
    }

    globalContextMenu->addAction(ui->selectAllAction);
    connect(ui->selectAllAction, SIGNAL(triggered()), this, SLOT(onSelectAll()));
    globalContextMenu->addAction(ui->keepOnlySelectionAction);
    globalContextMenu->addMenu(selectFromSizeMenu);
    globalContextMenu->addMenu(gotoFromOffsetMenu);
    connect(ui->keepOnlySelectionAction, SIGNAL(triggered()), this, SLOT(onKeepOnlySelection()));
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->newByteArrayAction);
    connect(ui->newByteArrayAction, SIGNAL(triggered()), this, SLOT(onNewByteArray()));
    globalContextMenu->addAction(ui->importFileAction);
    connect(ui->importFileAction, SIGNAL(triggered()), this, SLOT(onLoadFile()));
    globalContextMenu->addMenu(loadMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(replaceMenu);
    globalContextMenu->addMenu(insertAfterMenu);
    globalContextMenu->addMenu(insertBeforeMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(markMenu);
    globalContextMenu->addAction(ui->clearMarkingsAction);
    connect(ui->clearMarkingsAction, SIGNAL(triggered()), this, SLOT(onClearSelectionMarkings()));
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(saveToFileMenu);
    globalContextMenu->addMenu(copyMenu);
    globalContextMenu->addMenu(copySelectedSizeMenu);
    globalContextMenu->addMenu(copyCurrentOffsetMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addAction(ui->deleteSelectionAction);
    connect(ui->deleteSelectionAction, SIGNAL(triggered()), this, SLOT(onDeleteSelection()));
}
