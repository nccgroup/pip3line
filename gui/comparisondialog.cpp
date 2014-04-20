/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "comparisondialog.h"
#include "ui_comparisondialog.h"
#include <QVariant>
#include <QMessageBox>
#include "views/byteitemmodel.h"
#include "sources/bytesourceabstract.h"
#include "tabs/tababstract.h"
#include "views/bytetableview.h"
#include "loggerwidget.h"
#include "guihelper.h"
#include <QColorDialog>
#include <QPixmap>
#include <QComboBox>
#include <QThread>
#include <QDebug>

CompareWorker::CompareWorker(ByteSourceAbstract *sA, ByteSourceAbstract *sB, QObject *parent) :
    QObject(parent)
{
    sourceA = sA;
    connect(this, SIGNAL(markingA(quint64,quint64,QColor,QColor,QString)),
            sourceA, SLOT(mark(quint64,quint64,QColor,QColor,QString)),Qt::QueuedConnection);
    sourceB = sB;
    connect(this, SIGNAL(markingB(quint64,quint64,QColor,QColor,QString)),
            sourceB, SLOT(mark(quint64,quint64,QColor,QColor,QString)), Qt::QueuedConnection);
    startA = 0;
    startB = 0;
    sizeA = sourceA->size();
    sizeB = sourceB->size();
    markA = true;
    markB = true;
    markSame = false;
    stopped = false;
}

CompareWorker::~CompareWorker()
{

}

void CompareWorker::compare()
{
    qDebug() << "Start comparison" << startA << startB;
    bool equals = true;
    quint64 compSize = qMin(sizeA, sizeB);
    quint64 progressMark = compSize * 2 / 100;
    QString tooltipDiffA = tr("%1 \"%2\"").arg(markSame ? "Same as" : "Different from").arg(nameB);
    QString tooltipDiffB = tr("%1 \"%2\"").arg(markSame ? "Same as" : "Different from").arg(nameA);
    quint64 realA = 0;
    quint64 realB = 0;
    for (quint64 i = 0; i < compSize; i++) {
        realA = i + startA;
        realB = i + startB;
        if (sourceA->extract(realA) != sourceB->extract(realB)) {
            if (!markSame) {
                if (markA)
                    emit markingA(realA,realA,marksColor,QColor(),tooltipDiffA);
                if (markB)
                    emit markingB(realB,realB,marksColor,QColor(),tooltipDiffB);
            }
            equals = false;
        } else if (markSame) {
            if (markA)
                emit markingA(realA,realA,marksColor,QColor(),tooltipDiffA);
            if (markB)
                emit markingB(realB,realB,marksColor,QColor(),tooltipDiffB);

        }
        if (stopped) {
            emit finishComparing(equals);
            deleteLater();
            return;
        }
        if (i > 0 && progressMark > 0 && i % progressMark == 0) {
            emit progress((int)(float)(i) * ((float)100/(float)compSize));
        }
    }

    if (sizeA != sizeB && !markSame) {
        if (sizeA > sizeB) {
            if (markA)
                emit markingA(sizeB + startA,sizeA + startA -1,marksColor,QColor(),tooltipDiffA);
        } else {
            if (markB)
                emit markingB(sizeA + startB,sizeB + startB -1,marksColor,QColor(),tooltipDiffB);
        }
        equals = false;
    }

    emit finishComparing(equals);
}

void CompareWorker::stop()
{
    stopped = true;
}
QColor CompareWorker::getMarkColor() const
{
    return marksColor;
}

void CompareWorker::setMarkColor(const QColor &value)
{
    marksColor = value;
}

QString CompareWorker::getNameB() const
{
    return nameB;
}

void CompareWorker::setNameB(const QString &value)
{
    nameB = value;
}

QString CompareWorker::getNameA() const
{
    return nameA;
}

void CompareWorker::setNameA(const QString &value)
{
    nameA = value;
}

quint64 CompareWorker::getEndB() const
{
    return endB;
}

quint64 CompareWorker::getEndA() const
{
    return endA;
}

bool CompareWorker::getMarkSame() const
{
    return markSame;
}

void CompareWorker::setMarkSame(bool value)
{
    markSame = value;
}

bool CompareWorker::getMarkB() const
{
    return markB;
}

void CompareWorker::setMarkB(bool value)
{
    markB = value;
}

bool CompareWorker::getMarkA() const
{
    return markA;
}

void CompareWorker::setMarkA(bool value)
{
    markA = value;
}

quint64 CompareWorker::getSizeB() const
{
    return sizeB;
}

quint64 CompareWorker::getSizeA() const
{
    return sizeA;
}

quint64 CompareWorker::getStartB() const
{
    return startB;
}

void CompareWorker::setBRange(const quint64 start, const quint64 end)
{
    qDebug() << "B range" << start << end;
    if (start < end) {
        startB = start;
        endB = end;
    } else {
        startB = end;
        endB = start;
    }
    sizeB = end - start + 1;
}

quint64 CompareWorker::getStartA() const
{
    return startA;
}

void CompareWorker::setARange(const quint64 start, const quint64 end)
{
    qDebug() << "A range" << start << end;
    if (start < end) {
        startA = start;
        endA = end;
    } else {
        startA = end;
        endA = start;
    }

    sizeA = end - start + 1;
}


const QColor ComparisonDialog::DEFAULT_MARKING_COLOR = QColor(255,182,117);

ComparisonDialog::ComparisonDialog(GuiHelper *nguiHelper, QWidget *parent) :
    AppDialog(nguiHelper, parent)
{
    ui = new(std::nothrow) Ui::ComparisonDialog();
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::ComparisonDialog X{");
    }

    workerThread = NULL;
    ui->setupUi(this);

    marksColor = DEFAULT_MARKING_COLOR;
    changeIconColor(marksColor);

    ui->advancedWidget->hide();

    connect(guiHelper, SIGNAL(tabsUpdated()), SLOT(loadTabs()));
    connect(ui->tabAComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onTabSelection(int)));
    connect(ui->tabBComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onTabSelection(int)));
    connect(ui->entryAComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onEntrySelected(int)));
    connect(ui->entryBComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onEntrySelected(int)));
    connect(this, SIGNAL(finished(int)), this, SLOT(hide()));
    connect(ui->acceptPushButton, SIGNAL(clicked()), SLOT(onCompare()));
    connect(ui->colorPushButton, SIGNAL(clicked()), SLOT(oncolorChange()));
    connect(ui->advancedPushButton, SIGNAL(toggled(bool)), SLOT(onAdvancedClicked(bool)));
    loadTabs();
    //qDebug() << "Created" << this;

}

ComparisonDialog::~ComparisonDialog()
{
    //qDebug() << "Destroying " << this;
    tabs.clear();
    delete ui;
    guiHelper = NULL;
    if (workerThread != NULL) {
        workerThread->quit();
        if (!workerThread->wait(10000))
            qCritical() << "Could not stop the comparison thread.";
        delete workerThread;
    }
   // qDebug() << "Destroyed " << this;
}

void ComparisonDialog::onTabSelection(int index)
{
    if (index < 0)
        return; // nothing to do here

    QObject *tabCombo = sender();
    QComboBox * entryCombo = NULL;
    if (tabCombo == ui->tabAComboBox) {
        entryCombo = ui->entryAComboBox;
    } else {
        entryCombo = ui->entryBComboBox;
    }

    if (index < tabs.size()) {
        refreshEntries(entryCombo, tabs.at(index)->getBlockCount());
        connect(tabs.at(index),SIGNAL(entriesChanged()), SLOT(onTabEntriesChanged()),Qt::UniqueConnection);
    } else {
        guiHelper->getLogger()->logError(tr("Invalid index for tabs %1").arg(index));
    }
}

void ComparisonDialog::onTabEntriesChanged()
{
    TabAbstract *tab = dynamic_cast<TabAbstract *>(sender());
    if (tab == NULL) {
        qWarning() << "[ComparisonDialog::onTabEntriesChanged] NULL tab";
    }

    int index = tabs.indexOf(tab);
    if (index != -1) {
        if (ui->tabAComboBox->currentIndex() == index) {
            refreshEntries(ui->entryAComboBox, tabs.at(index)->getBlockCount());
        }

        if (ui->tabBComboBox->currentIndex() == index) {
            refreshEntries(ui->entryBComboBox, tabs.at(index)->getBlockCount());
        }
    } else {
       guiHelper->getLogger()->logError(tr("Tab not found for entries changed").arg(index));
    }
}

void ComparisonDialog::onCompare()
{
    TabAbstract *tabA = tabs.at(ui->tabAComboBox->currentIndex());
    TabAbstract *tabB = tabs.at(ui->tabBComboBox->currentIndex());

    ByteSourceAbstract * sourceA = tabA->getSource(ui->entryAComboBox->currentIndex());
    ByteSourceAbstract * sourceB = tabB->getSource(ui->entryBComboBox->currentIndex());
    if (sourceA == NULL || sourceB == NULL)
        return;

    CompareWorker *worker = new(std::nothrow) CompareWorker(sourceA, sourceB);
    if (worker == NULL) {
        qFatal("Cannot allocate memory for CompareWorker X{");
    }

    if (ui->selectedCheckBox->isChecked()) {
        worker->setARange(sourceA->getRealOffset(tabA->getHexTableView(ui->entryAComboBox->currentIndex())->getLowerSelected()),
                          sourceA->getRealOffset(tabA->getHexTableView(ui->entryAComboBox->currentIndex())->getHigherSelected())
                          );
        worker->setBRange(sourceB->getRealOffset(tabB->getHexTableView(ui->entryBComboBox->currentIndex())->getLowerSelected()),
                         sourceB->getRealOffset(tabB->getHexTableView(ui->entryBComboBox->currentIndex())->getHigherSelected())
                         );
    }

    worker->setMarkA(ui->markACheckBox->isChecked());
    worker->setMarkB(ui->markBCheckBox->isChecked());
    worker->setMarkSame(ui->sameRadioButton->isChecked());
    worker->setNameA(tabA->getName());
    worker->setNameB(tabB->getName());
    worker->setMarkColor(marksColor);

    if (ui->clearACheckBox->isChecked()) {
        sourceA->clearAllMarkings();
    }

    if (ui->clearBCheckBox->isChecked()) {
        sourceB->clearAllMarkings();
    }
    workerThread = new(std::nothrow) QThread();
    if (workerThread == NULL) {
        qFatal("Cannot allocate memory for workerThread X{");
    }
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()), worker, SLOT(compare()));
  //  connect(workerThread, SIGNAL(finished()), worker,SLOT(deleteLater()));
    connect(worker,SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
    connect(worker,SIGNAL(finishComparing(bool)), SLOT(endOfComparison(bool)));

    ui->stackedWidget->setCurrentIndex(1);
    workerThread->start();
}

void ComparisonDialog::loadTabs()
{
    tabs = guiHelper->getTabs();
    if (tabs.size() > 0) {
        setEnabled(true);
        refreshTabs(ui->tabAComboBox);
        refreshTabs(ui->tabBComboBox);

        refreshEntries(ui->entryAComboBox,tabs.at(ui->tabAComboBox->currentIndex())->getBlockCount());
        refreshEntries(ui->entryBComboBox,tabs.at(ui->tabBComboBox->currentIndex())->getBlockCount());
    } else {
        setDisabled(true);
        ui->tabAComboBox->blockSignals(true);
        ui->tabAComboBox->clear();
        ui->tabAComboBox->blockSignals(false);
        ui->tabBComboBox->blockSignals(true);
        ui->tabBComboBox->clear();
        ui->tabAComboBox->blockSignals(false);
        ui->entryAComboBox->blockSignals(true);
        ui->entryAComboBox->clear();
        ui->entryAComboBox->blockSignals(false);
        ui->entryBComboBox->blockSignals(true);
        ui->entryBComboBox->clear();
        ui->entryBComboBox->blockSignals(false);
    }

}

void ComparisonDialog::oncolorChange()
{
    marksColor = QColorDialog::getColor(Qt::yellow, this);
    changeIconColor(marksColor);
}

void ComparisonDialog::onAdvancedClicked(bool status)
{
    ui->advancedWidget->setVisible(status);
    adjustSize();
}

void ComparisonDialog::onEntrySelected(int index)
{
    int indexA = ui->entryAComboBox->currentIndex();
    int indexB = ui->entryBComboBox->currentIndex();
    if (indexA < 0 || indexB < 0) {
        return; // nothing to see here
    }

    TabAbstract *tabA = tabs.at(ui->tabAComboBox->currentIndex());
    TabAbstract *tabB = tabs.at(ui->tabBComboBox->currentIndex());
    ByteSourceAbstract * sourceA = NULL;
    ByteSourceAbstract * sourceB = NULL;

    if (sender() == ui->entryAComboBox) {
        indexA = index;
    } else {
        indexB = index;
    }

    sourceA = tabA->getSource(indexA);
    sourceB = tabB->getSource(indexB);

    if (sourceA == NULL || sourceB == NULL)
        return;

    if (sourceA->hasCapability(ByteSourceAbstract::CAP_COMPARE) && sourceB->hasCapability(ByteSourceAbstract::CAP_COMPARE)) {
        ui->selectedCheckBox->setEnabled(true);
        ui->selectedCheckBox->setToolTip("");
    } else {
        ui->selectedCheckBox->setChecked(true);
        ui->selectedCheckBox->setEnabled(false);
        ui->selectedCheckBox->setToolTip(tr("One of the samples is not suitable for comparison"));
    }

}

void ComparisonDialog::endOfComparison(bool equals)
{
    if (equals) {
        QMessageBox::warning(this, tr("No difference"), tr("No difference was found"), QMessageBox::Ok);
    }
    workerThread->quit();
    workerThread->deleteLater();
    workerThread = NULL;
    ui->stackedWidget->setCurrentIndex(0);
}

void ComparisonDialog::refreshEntries(QComboBox *entryBox, int count)
{
    entryBox->clear();
    for (int i = 1; i <= count; i++){
        entryBox->addItem(tr("%1").arg(i));
    }
}

void ComparisonDialog::refreshTabs(QComboBox *tabBox)
{
    tabBox->blockSignals(true);
    tabBox->clear();
    for (int i = 0; i < tabs.size(); i++) {
        tabBox->addItem(tabs.at(i)->getName());
    }
    if (tabs.size() > 0) {
        connect(tabs.at(tabBox->currentIndex()),SIGNAL(entriesChanged()), SLOT(onTabEntriesChanged()),Qt::UniqueConnection);
    }
    tabBox->blockSignals(false);
}

void ComparisonDialog::changeIconColor(QColor color)
{
    QPixmap pix(20,20);
    pix.fill(color);
    ui->colorPushButton->setIcon(QIcon(pix));
}

