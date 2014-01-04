#include "memorywidget.h"
#include "currentmemorysource.h"
#include <QPoint>
#include <QDebug>
#include <QMenu>
#include <QTableWidgetItem>
#include <QList>
#include <QAction>
#include <QModelIndex>
#include "ui_memorywidget.h"
#include "../shared/searchwidget.h"

const QString MemoryWidget::GOTOSTART = "Go to start of range";
const QString MemoryWidget::GOTOEND = "Go to end of range";

MemoryWidget::MemoryWidget(CurrentMemorysource *source, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MemoryWidget)
{
    msource = source;
    ui->setupUi(this);

    SearchWidget *searchWidget = new(std::nothrow) SearchWidget(msource,this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    ui->memLayout->insertWidget(0, searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,bool)), SLOT(onSearch(QByteArray,bool)));

    ui->mappingsTableWidget->verticalHeader()->setDefaultSectionSize(25);

    MemRangeModel * ranges = msource->getMemRanges();
    ui->mappingsTableWidget->setModel(ranges);
    ui->mappingsTableWidget->resizeColumnsToContents();
    ui->mappingsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mappingsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->mappingsTableWidget, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClick(QModelIndex)));

    ui->refreshCheckBox->setChecked(msource->isRefreshEnabled());
    ui->refreshRateSpinBox->setValue(msource->refreshInterval());
    ui->refreshRateSpinBox->setEnabled(msource->isRefreshEnabled());

    initContextMenu();
    connect(ui->refreshCheckBox,SIGNAL(toggled(bool)), SLOT(onRefreshToggled(bool)));
}

MemoryWidget::~MemoryWidget()
{
    delete ui;
}

void MemoryWidget::setProcSelection(bool val)
{
    ui->procSelectWidget->setVisible(val);
}

void MemoryWidget::onDoubleClick(QModelIndex index)
{
    if (index.isValid()) {
        MemRange * range = msource->getMemRanges()->getRange(index);
        if (range != NULL) {
            msource->setStartingOffset(range->getLowerVal());
        }
    }
}

void MemoryWidget::onRefreshToggled(bool val)
{
    ui->refreshRateSpinBox->setEnabled(val);
    msource->setRefreshEnable(val);
    if (val)
        msource->setTimerInterval(ui->refreshRateSpinBox->value());
}

void MemoryWidget::onRightClick(QPoint pos)
{
    contextMenu->exec(this->mapToGlobal(pos));
}

void MemoryWidget::contextMenuAction(QAction *action)
{
    QModelIndexList list = ui->mappingsTableWidget->selectionModel()->selectedIndexes();

    if (list.size() < 1)
        return;

    MemRange * range = msource->getMemRanges()->getRange(list.at(0));
    quint64 offset = range->getLowerVal();
    if (action->text() != GOTOSTART) {
        quint64 endoffset = range->getUpperVal();
        if (!(endoffset  + 1 - msource->viewSize() < offset))
            offset = endoffset + 1 - msource->viewSize();
    }
    msource->setStartingOffset(offset);
}

void MemoryWidget::onSearch(QByteArray item, bool maybetext)
{
    //todo
}

void MemoryWidget::initContextMenu()
{

    contextMenu = new(std::nothrow) QMenu();
    if (contextMenu == NULL) {
        qFatal("Cannot allocate memory for the Memory contextMenu X{");
    }

    connect(contextMenu, SIGNAL(triggered(QAction*)), SLOT(contextMenuAction(QAction*)));
    QAction * action = new(std::nothrow) QAction(GOTOSTART,contextMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for GOTOSTART X{");
        return;
    }

    contextMenu->addAction(action);

    action = new(std::nothrow) QAction(GOTOEND,contextMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for GOTOEND X{");
        return;
    }
    contextMenu->addAction(action);

    ui->mappingsTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mappingsTableWidget,SIGNAL(customContextMenuRequested(QPoint)), SLOT(onRightClick(QPoint)));
}
