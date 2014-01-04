#include "filewidget.h"
#include "ui_filewidget.h"
#include "largefile.h"
#include "../shared/searchwidget.h"
#include "../guihelper.h"
#include <QFileInfo>
#include <QTimer>
#include <QTimerEvent>
#include <QDebug>

FileWidget::FileWidget(LargeFile *fsource, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileWidget)
{
    source = fsource;
    ui = new(std::nothrow) Ui::FileWidget;
    ui->setupUi(this);

    itemsFoundModel = new(std::nothrow) FoundOffsetsModel();
    if (itemsFoundModel == NULL) {
        qFatal("Cannot allocate memory for SearchItemModel X{");
    }

    ui->itemsFoundListView->setModel(itemsFoundModel);

    searchWidget = new(std::nothrow) SearchWidget(fsource,this);
    if (searchWidget == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    searchWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    ui->mainLayout->insertWidget(ui->mainLayout->indexOf(ui->statsWidget) + 1, searchWidget);
    connect(searchWidget, SIGNAL(searchRequest(QByteArray,bool)), SLOT(onSearch(QByteArray,bool)));

    multiSearch = new(std::nothrow) FileSearch(source->fileName());
    if (multiSearch == NULL) {
        qFatal("Cannot allocate memory for SearchWidget X{");
    }
    connect(multiSearch, SIGNAL(searchStarted()), searchWidget, SLOT(onSearchStarted()));
    connect(multiSearch, SIGNAL(searchEnded()), searchWidget, SLOT(onSearchEnded()));
    connect(multiSearch, SIGNAL(searchEnded()), SLOT(onEndSearch()));
    connect(multiSearch, SIGNAL(progressUpdate(quint64)), searchWidget, SLOT(updateProgress(quint64)));
    connect(multiSearch, SIGNAL(itemFound(quint64,quint64)), SLOT(itemFound(quint64,quint64)));
    connect(searchWidget, SIGNAL(stopSearch()), multiSearch, SLOT(stopSearch()));

    refresh();
    connect(source,SIGNAL(infoUpdated()), SLOT(refresh()));
    connect(ui->refreshPushButton, SIGNAL(clicked()), SLOT(refresh()));
    connect(ui->itemsFoundListView, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClick(QModelIndex)));
}

FileWidget::~FileWidget()
{
    delete ui;
    delete multiSearch;
}

void FileWidget::onSearch(QByteArray item, bool)
{
    itemsFoundModel->clear();
    source->clearAllMarkings();
    multiSearch->setSearchItem(item);
    multiSearch->startThreadedSearch();
}

void FileWidget::refresh()
{
    QFileInfo info(source->fileName());
//    if (parentWidget() != NULL && QString(parentWidget()->metaObject()->className()) == "FloatingDialog")
//        parentWidget()->setWindowTitle(source->description());
    ui->pathTextLabel->setText(info.absoluteFilePath());
    ui->sizeTextLabel->setText(QString::number(info.size()).append(" bytes"));
    QString perms;
    if (info.isReadable()) {
        perms.append("Readable\n");
    }

    if (info.isWritable())
        perms.append("Writable\n");

    if (info.isExecutable())
        perms.append("Executable\n");

    if (info.isSymLink())
        perms.append("Symlink\n");

    if (info.isFile())
        perms.append("Normal file");
    else if (info.isSymLink())
        perms.append("symbolic link");
    else if (info.isDir())
        perms.append("Directory");

    if (info.isHidden())
        perms.append("Hidden file");

    ui->rightsTextLabel->setText(perms);
    searchWidget->clearSearch();
    itemsFoundModel->clear();
}

void FileWidget::itemFound(quint64 soffset, quint64 eoffset)
{
    itemsFoundModel->addOffset(soffset, eoffset);
    source->mark(soffset, eoffset, QColor(255,182,117));
}

void FileWidget::onDoubleClick(QModelIndex index)
{
    quint64 offset = itemsFoundModel->getStartingOffset(index);
    source->setStartingOffset(offset);
}

void FileWidget::onEndSearch()
{
    if (itemsFoundModel->rowCount() > 0)
        ui->resultLabel->setText(QString::number(itemsFoundModel->rowCount()));
    else
        ui->resultLabel->setText(tr("Nothing found"));
}

