/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "interceptwidget.h"
#include "ui_interceptwidget.h"
#include "intercepsource.h"
#include "../blocksources/udplistener.h"
#include "../blocksources/tcpserverlistener.h"
#include <QTableView>
#include <QStandardItem>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include "../blocksources/rawtcplistener.h"

const QString InterceptWidget::CHOOSE_TEXT = QObject::tr("Choose source");
const QString InterceptWidget::UDP_EXTERNAL_SOURCE_TEXT = QObject::tr("External process (UDP)");
const QString InterceptWidget::TCP_EXTERNAL_SOURCE_TEXT = QObject::tr("External process (TCP)");
const QString InterceptWidget::RAW_TCP_SOURCE_TEXT = QObject::tr("Raw TCP client");
const QFont InterceptWidget::RegularFont = QFont("Courier New",10);
const int InterceptWidget::TIMESTAMP_COLUMN_WIDTH = 135;

InterceptWidget::InterceptWidget(IntercepSource * source, QWidget *parent) :
    QWidget(parent),
    source(source)
{
    currentBlockSource = NULL;
    currentGui = NULL;
    ui = new(std::nothrow) Ui::InterceptWidget;
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::InterceptWidget X{");
    }
    ui->setupUi(this);

    packetsTable = new(std::nothrow) QTableView(this);
    if (packetsTable == NULL) {
        qFatal("Cannot allocate memory for QTableView X{");
    }
    QAbstractItemModel *old = packetsTable->model();
    model = source->getModel();
    packetsTable->setModel(model);
    delete old;

    packetsTable->setSelectionMode(QAbstractItemView::ContiguousSelection);
    packetsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    packetsTable->verticalHeader()->setFont(RegularFont);
    packetsTable->horizontalHeader()->setFont(RegularFont);
    packetsTable->setColumnWidth(PayloadModel::TIMESPTAMP_COLUMN,TIMESTAMP_COLUMN_WIDTH);
    packetsTable->setColumnWidth(PayloadModel::DIRECTION_COLUMN,25);
    packetsTable->verticalHeader()->setDefaultSectionSize(20);
#if QT_VERSION >= 0x050000
    packetsTable->horizontalHeader()->setSectionsMovable( false );
#else
    packetsTable->horizontalHeader()->setMovable(true);
#endif
    connect(packetsTable->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentSelectedChanged(QModelIndex,QModelIndex)));
    ui->listLayout->addWidget(packetsTable);

    updateColumns();

    sourceChoices << CHOOSE_TEXT << UDP_EXTERNAL_SOURCE_TEXT << TCP_EXTERNAL_SOURCE_TEXT << RAW_TCP_SOURCE_TEXT;
    ui->blockSourceComboBox->addItems(sourceChoices);
    ui->blockSourceComboBox->setCurrentIndex(0);
    QStandardItem * item = qobject_cast<QStandardItemModel *>(ui->blockSourceComboBox->model())->item(0);
    item->setEnabled( false );
    item->setTextAlignment(Qt::AlignCenter);
    item->setBackground(Qt::darkGray);
    item->setForeground(Qt::white);

    connect(ui->blockSourceComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(onSourceChanged(QString)));
}

InterceptWidget::~InterceptWidget()
{
    delete ui;
    model = NULL;
}

void InterceptWidget::onSourceChanged(QString selected)
{
    if (selected.compare(CHOOSE_TEXT) == 0) {
        qWarning() << "[InterceptWidget::onSourceChanged] cannot select CHOOSE_TEXT as a source T_T";
        return; // nothing to do here
    } else if (selected.compare(UDP_EXTERNAL_SOURCE_TEXT) == 0) {
        if (currentBlockSource != NULL) {
            delete currentBlockSource;
        }
        currentBlockSource = new(std::nothrow) UdpListener();
        if (currentBlockSource == NULL) {
            qFatal("Cannot allocate memory for UdpListener X{");
        }
    } else if (selected.compare(TCP_EXTERNAL_SOURCE_TEXT) == 0) {
        if (currentBlockSource != NULL) {
            delete currentBlockSource;
        }
        currentBlockSource = new(std::nothrow) TcpServerListener();
        if (currentBlockSource == NULL) {
            qFatal("Cannot allocate memory for TcpServerListener X{");
        }
    } else if (selected.compare(RAW_TCP_SOURCE_TEXT) == 0) {
        currentBlockSource = new(std::nothrow) RawTcpListener();
        if (currentBlockSource == NULL) {
            qFatal("Cannot allocate memory for RawTcpListener X{");
        }
    } else { // idiot proof
        qFatal("[InterceptWidget::onSourceChanged] Selection not managed T_T");
    }

    if (currentBlockSource != NULL) {
        if (currentGui != NULL) {
            delete currentGui;
        }
        currentGui = currentBlockSource->getGui(this);
        if (currentGui != NULL) {
            ui->sourceLayout->insertWidget(1,currentGui);

            connect(currentGui, SIGNAL(destroyed()), SLOT(onCurrentGuiDeleted()), Qt::DirectConnection);
        }
        source->setCurrentBlockSource(currentBlockSource);
    }
}

void InterceptWidget::onCurrentGuiDeleted()
{
    currentGui = NULL;
    qDebug() << "Gui deleted";
}

void InterceptWidget::updateColumns()
{
    QStringList columnList = model->getColumnNames();
    QStandardItemModel *columnModel = new(std::nothrow) QStandardItemModel(columnList.size(), 1);
    if (columnModel == NULL ) {
        qFatal("Cannot allocate memory for QStandardItemModel X{");
        return;
    }

    for (int i = 0; i < columnList.size(); ++i)
    {
        QStandardItem* item = NULL;
        item = new(std::nothrow) QStandardItem(columnList.at(i));
        if (item != NULL) {
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData( (packetsTable->isColumnHidden(i) ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
            columnModel->setItem(i, 0, item);
        } else {
          qFatal("Cannot allocate memory for QStandardItem 2 X{");
        }
    }
    connect(columnModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onColumnChanged(QModelIndex,QModelIndex)));
    ui->columnListWidget->setModel(columnModel);
}

void InterceptWidget::onCurrentSelectedChanged(const QModelIndex &topLeft, const QModelIndex &)
{
    if (topLeft.isValid()) {
        int index = topLeft.row();

        source->configureCurrentPayload(model->getPayload(index));
    }
}

void InterceptWidget::onColumnChanged(const QModelIndex &topLeft, const QModelIndex &)
{
    int index = topLeft.row();
    Qt::CheckState state = (Qt::CheckState) topLeft.data(Qt::CheckStateRole).toInt();
    if (state == Qt::Checked) {
        packetsTable->showColumn(index);
    } else {
        packetsTable->hideColumn(index);
    }
}

