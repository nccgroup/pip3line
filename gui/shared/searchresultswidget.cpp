#include "searchresultswidget.h"
#include "searchwidget.h"
#include <QAbstractListModel>
#include <QAction>
#include <QMenu>
#include <QItemSelectionModel>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_searchresultswidget.h"
#include "../sources/searchabstract.h"
#include "../sources/bytesourceabstract.h"
#include "guiconst.h"
using namespace GuiConst;

SearchResultsWidget::SearchResultsWidget(FoundOffsetsModel * offsetModel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchResultsWidget)
{
    saveToFileMenu = NULL;
    saveToClipboardMenu = NULL;
    ui->setupUi(this);
    itemModel = offsetModel;
    if (itemModel->rowCount() > 0) {
        ui->clearResultsPushButton->setEnabled(true);
        ui->savePushButton->setEnabled(true);
    } else {
        ui->clearResultsPushButton->setEnabled(false);
        ui->savePushButton->setEnabled(false);
    }
    QAbstractItemModel * oldModel = ui->listView->model();
    ui->listView->setModel(itemModel);
    ui->listView->setUniformItemSizes(true);
    ui->listView->setLayoutMode(QListView::Batched);
    ui->listView->setFont(GuiStyles::DEFAULT_REGULAR_FONT);
    delete oldModel;

    QAction * action = NULL;

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == NULL) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
    }

    copyAsHexadecimal = new(std::nothrow) QAction(HEXADECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == NULL) {
        qFatal("Cannot allocate memory for HEXADECIMAL X{");
    }

    copyAsDecimal = new(std::nothrow) QAction(DECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == NULL) {
        qFatal("Cannot allocate memory for DECIMAL X{");
    }

    saveListToFileAsHexadecimal = new(std::nothrow) QAction(HEXADECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == NULL) {
        qFatal("Cannot allocate memory for HEXADECIMAL X{");
    }

    saveListToFileAsDecimal = new(std::nothrow) QAction(DECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == NULL) {
        qFatal("Cannot allocate memory for DECIMAL X{");
    }

    copyListAsHexadecimal = new(std::nothrow) QAction(HEXADECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == NULL) {
        qFatal("Cannot allocate memory for HEXADECIMAL X{");
    }

    copyListAsDecimal = new(std::nothrow) QAction(DECIMAL_STRING, globalContextMenu);
    if (copyAsHexadecimal == NULL) {
        qFatal("Cannot allocate memory for DECIMAL X{");
    }

    action = new(std::nothrow) QAction(COPYOFFSET, globalContextMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for COPYOFFSET X{");
    }
    action->setDisabled(true);
    globalContextMenu->addAction(action);
    globalContextMenu->addAction(copyAsHexadecimal);
    globalContextMenu->addAction(copyAsDecimal);

    onModelUpdated();

    saveToFileMenu = new(std::nothrow) QMenu(tr("Save to file"),ui->savePushButton);
    if (saveToFileMenu == NULL) {
        qFatal("Cannot allocate memory for saveToFileMenu X{");
    }
    saveToFileMenu->addAction(saveListToFileAsHexadecimal);
    saveToFileMenu->addAction(saveListToFileAsDecimal);
    ui->savePushButton->setMenu(saveToFileMenu);

    saveToClipboardMenu = new(std::nothrow) QMenu(tr("Save to clipboard"),ui->savePushButton);
    if (saveToClipboardMenu == NULL) {
        qFatal("Cannot allocate memory for saveToClipboardMenu X{");
    }

    saveToClipboardMenu->addAction(copyListAsHexadecimal);
    saveToClipboardMenu->addAction(copyListAsDecimal);
    ui->copyPushButton->setMenu(saveToClipboardMenu);

    connect(ui->listView,SIGNAL(customContextMenuRequested(QPoint)), SLOT(onRightClick(QPoint)));
    connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClick(QModelIndex)));
    connect(globalContextMenu, SIGNAL(triggered(QAction*)), SLOT(contextMenuAction(QAction*)));
    connect(itemModel, SIGNAL(updated()), SLOT(onModelUpdated()));
    connect(ui->clearResultsPushButton, SIGNAL(clicked()), SLOT(onClear()));
    connect(saveListToFileAsHexadecimal, SIGNAL(triggered()), SLOT(onSaveToFileHexAction()));
    connect(saveListToFileAsDecimal, SIGNAL(triggered()), SLOT(onSaveToFileDecAction()));
}

SearchResultsWidget::~SearchResultsWidget()
{
    delete ui;
    itemModel = NULL;
}

void SearchResultsWidget::clearResults()
{
    itemModel->clear();
}

void SearchResultsWidget::onDoubleClick(QModelIndex index)
{
    emit jumpTo(itemModel->getStartingOffset(index), itemModel->getEndOffset(index));
}

void SearchResultsWidget::onModelUpdated()
{
    if (itemModel->rowCount() > 0) {
        ui->resultsLabel->setText(tr("Results: %1 (%2 ms)").arg(itemModel->rowCount()).arg(itemModel->elapsed()));
        ui->clearResultsPushButton->setEnabled(true);
        ui->savePushButton->setEnabled(true);
        ui->copyPushButton->setEnabled(true);
    } else {
        ui->resultsLabel->setText(tr("No results"));
        ui->clearResultsPushButton->setEnabled(false);
        ui->savePushButton->setEnabled(false);
        ui->copyPushButton->setEnabled(false);
    }

    ui->resultsLabel->setVisible(true);
}

void SearchResultsWidget::onRightClick(QPoint pos)
{
    globalContextMenu->exec(this->mapToGlobal(pos));
}

void SearchResultsWidget::contextMenuAction(QAction *action)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text;
    QModelIndex index;
    if (action == copyAsHexadecimal) {
        index = ui->listView->selectionModel()->currentIndex();
        clipboard->setText(index.data().toString());
    } else if (action == copyAsDecimal) {
        index = ui->listView->selectionModel()->currentIndex();
        clipboard->setText(QString::number(itemModel->getStartingOffset(index)));
    } else if (action == copyListAsHexadecimal) {
        BytesRangeList * list = itemModel->getRanges();
        for (int i = 0; i < list->size(); i++) {
            text.append(tr("0x%1\n").arg(list->at(i)->getLowerVal(), 0, 16));
        }
        text.chop(1);
        clipboard->setText(text);
    } else if (action == copyListAsDecimal) {
        BytesRangeList * list = itemModel->getRanges();
        for (int i = 0; i < list->size(); i++) {
            text.append(tr("%1\n").arg(list->at(i)->getLowerVal()));
        }
        text.chop(1);
        clipboard->setText(text);
    }
}

void SearchResultsWidget::onClear()
{
    itemModel->clear();
    ui->resultsLabel->setText(tr("Results: 0"));
}

void SearchResultsWidget::savetoFile(QString format, int base)
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose a file to save to"));
    if (!fileName.isEmpty()) {
        BytesRangeList * list = itemModel->getRanges();
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::critical(this, tr("Failed to open file"), file.errorString());
            return;
        }
        for (int i = 0; i < list->size(); i++) {
            qint64 written = 0;
            QByteArray data = format.arg(list->at(i)->getLowerVal(), 0, base).toUtf8();
            while ((written = file.write(data)) > 0) {
                if (written == data.length())
                    break;
                else
                    data = data.mid(written - 1);
            }
            if (i < list->size() - 1)
                file.write("\n");
        }

        file.close();
    }
}

void SearchResultsWidget::onSaveToFileHexAction()
{
    savetoFile("0x%1",16);
}

void SearchResultsWidget::onSaveToFileDecAction()
{
    savetoFile("%1",10);
}
