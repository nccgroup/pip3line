#include "comparisondialog.h"
#include "ui_comparisondialog.h"
#include <QVariant>
#include <QMessageBox>
#include "views/byteitemmodel.h"
#include "sources/bytesourceabstract.h"
#include <QColorDialog>
#include <QPixmap>
#include <QDebug>

const QColor ComparisonDialog::DEFAULT_MARKING_COLOR = QColor(255,182,117);

ComparisonDialog::ComparisonDialog(GuiHelper *nguiHelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComparisonDialog)
{
    guiHelper = nguiHelper;
    ui->setupUi(this);

    marksColor = DEFAULT_MARKING_COLOR;
    changeIconColor(marksColor);

    connect(guiHelper, SIGNAL(tabsUpdated()), SLOT(loadTabs()));
    connect(ui->tabAComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onTabSelection(int)));
    connect(ui->tabBComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onTabSelection(int)));
    connect(this, SIGNAL(finished(int)), this, SLOT(hide()));
    connect(ui->acceptPushButton, SIGNAL(clicked()), SLOT(onCompare()));
    connect(ui->colorPushButton, SIGNAL(clicked()), SLOT(oncolorChange()));
    loadTabs();
    qDebug() << "Created" << this;

}

ComparisonDialog::~ComparisonDialog()
{
    qDebug() << "Destroying " << this;
    tabs.clear();
    delete ui;
    guiHelper = NULL;
    qDebug() << "Destroyed " << this;
}

void ComparisonDialog::onTabSelection(int index)
{
    QObject *tabCombo = sender();
    QComboBox * entryCombo = NULL;
    if (tabCombo == ui->tabAComboBox) {
        entryCombo = ui->entryAComboBox;
    } else {
        entryCombo = ui->entryBComboBox;
    }

    if (index < tabs.size()) {
        refreshEntries(entryCombo, tabs.at(index)->getBlockCount());
    } else {
        guiHelper->getLogger()->logError(tr("Invalid index for tabs %1").arg(index));
    }
}

void ComparisonDialog::onCompare()
{
    ByteItemModel *modelA = tabs.at(ui->tabAComboBox->currentIndex())->getBytesModelFrom(ui->entryAComboBox->currentIndex());
    ByteItemModel *modelB = tabs.at(ui->tabBComboBox->currentIndex())->getBytesModelFrom(ui->entryBComboBox->currentIndex());

    ByteSourceAbstract * sourceA = modelA->getSource();
    ByteSourceAbstract * sourceB = modelB->getSource();

    int compSize = qMin(sourceA->size(), sourceB->size());
    bool equals = true;
    for (int i = 0; i < compSize; i++) {
        if (sourceA->extract(i) != sourceB->extract(i)) {
            modelA->mark(i,i,marksColor);
            modelB->mark(i,i,marksColor);
            equals = false;
        }
    }

    if (sourceA->size() != sourceB->size()) {
        if (sourceA->size() > sourceB->size()) {
            modelA->mark(sourceB->size() - 1,sourceA->size() -1,marksColor,tr("Different"));
        } else {
            modelB->mark(sourceA->size() - 1,sourceB->size() -1,marksColor,tr("Different"));
        }
        equals = false;
    }

    if (equals) {
        QMessageBox::warning(this, tr("No difference"), tr("No difference was found"), QMessageBox::Ok);
    }
}

void ComparisonDialog::loadTabs()
{
    tabs = guiHelper->getTabs();

    refreshTabs(ui->tabAComboBox);
    refreshTabs(ui->tabBComboBox);

    refreshEntries(ui->entryAComboBox,tabs.at(ui->tabAComboBox->currentIndex())->getBlockCount());
    refreshEntries(ui->entryBComboBox,tabs.at(ui->tabBComboBox->currentIndex())->getBlockCount());
}

void ComparisonDialog::oncolorChange()
{
    marksColor = QColorDialog::getColor(Qt::yellow, this);
    changeIconColor(marksColor);
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
    tabBox->blockSignals(false);
}

void ComparisonDialog::changeIconColor(QColor color)
{
    QPixmap pix(20,20);
    pix.fill(color);
    ui->colorPushButton->setIcon(QIcon(pix));
}

