/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <transformfactoryplugininterface.h>
#include "pluginconfwidget.h"
#include <QHashIterator>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QListWidgetItem>
#include <commonstrings.h>
#include <deleteablelistitem.h>
#include "quickviewitemconfig.h"
#include <QDebug>
using namespace Pip3lineConst;

const QString SettingsDialog::LOGID = "SettingsDialog";

SettingsDialog::SettingsDialog(GuiHelper *nhelper, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    helper = nhelper;
    tManager = helper->getTransformFactory();
    settings = tManager->getSettingsObj();
    ui->setupUi(this);

    ui->portSpinBox->setValue(helper->getDefaultPort());
    ui->decodeCheckBox->setChecked(helper->getDefaultServerDecode());
    ui->encodeCheckBox->setChecked(helper->getDefaultServerEncode());
    ui->pipeNameLineEdit->setText(helper->getDefaultServerPipeName());
    ui->autoUpdateCheckBox->setChecked(settings->value(SETTINGS_AUTO_UPDATE, true).toBool());
    ui->minimizeCheckBox->setChecked(settings->value(SETTINGS_MINIMIZE_TO_TRAY, true).toBool());
    ui->hexWidget->setChar(helper->getDefaultServerSeparator());

    connect(ui->autoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoUpdateChanged(bool)));
    connect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onMinimizeChanged(bool)));
    connect(ui->checkUpdatePushButton, SIGNAL(clicked()), this, SLOT(onUpdateRequest()));
    setModal(false);

    updatePluginList();
    updateRegisteredList();
    updateSavedMarkingColors();
    updateImportExportFuncs();

    connect(tManager, SIGNAL(savedUpdated()), this, SLOT(updateRegisteredList()));
    connect(helper, SIGNAL(markingsUpdated()), this, SLOT(updateSavedMarkingColors()));
    connect(ui->resetMarkingsPushButton, SIGNAL(clicked()), this, SLOT(onResetMarkings()));
    connect(helper, SIGNAL(importExportUpdated()), this, SLOT(updateImportExportFuncs()));
    connect(ui->resetIEFuncsPushButton, SIGNAL(clicked()), this, SLOT(onResetImportExportFuncs()));
    connect(ui->savedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onSavedClicked(QListWidgetItem*)));
    connect(ui->pluginsListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onPluginClicked(QModelIndex)));
    connect(ui->decodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerDecodeChanged(bool)));
    connect(ui->encodeCheckBox, SIGNAL(toggled(bool)), this, SLOT(onServerEncodeChanged(bool)));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onServerPortChanged(int)));
    connect(ui->pipeNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onServerPipeNameChanged(QString)));
    connect(ui->hexWidget, SIGNAL(charChanged(char)), this, SLOT(onServerSeparatorChanged(char)));
    connect(ui->importExportListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDoubleClickImportExportFuncs(QListWidgetItem*)));
    connect(ui->addImportExportPushButton, SIGNAL(clicked()), this, SLOT(onAddImportExportFuncs()));
}

SettingsDialog::~SettingsDialog()
{
    delete settings;
    delete ui;
}

void SettingsDialog::setVersionUpdateMessage(QString mess)
{
    ui->versionCheckedLabel->setText(mess);
}

void SettingsDialog::autoUpdateChanged(bool checked)
{
    settings->setValue(SETTINGS_AUTO_UPDATE, checked);
}


void SettingsDialog::onMinimizeChanged(bool checked)
{
    settings->setValue(SETTINGS_MINIMIZE_TO_TRAY, checked);
}

void SettingsDialog::onUpdateRequest()
{
    emit updateCheckRequested();
}

void SettingsDialog::updatePluginList()
{
    ui->pluginsListWidget->clear();
    stackedList.clear();
    int count = ui->pluginsStackedWidget->count();
    for (int i = 0 ; i < count; i++) {
      QWidget * wid = ui->pluginsStackedWidget->widget(i);
      ui->pluginsStackedWidget->removeWidget(wid);
      delete wid;
    }

    QHash<QString, TransformFactoryPluginInterface *> pluginList = tManager->getPlugins();

    QHashIterator<QString, TransformFactoryPluginInterface *> i(pluginList);
    while (i.hasNext()) {
        i.next();
        PluginConfWidget *widget = new PluginConfWidget(i.value());
        stackedList.insert(i.key(), ui->pluginsStackedWidget->addWidget(widget));
        ui->pluginsListWidget->addItem(i.key());
    }
    ui->pluginsListWidget->sortItems();
}

void SettingsDialog::updateRegisteredList()
{
    ui->savedListWidget->clear();
    QStringList list = tManager->getSavedConfs().keys();

    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new DeleteableListItem(list.at(i));
        connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onDeleteSaved(QString)));
        QListWidgetItem *item = new QListWidgetItem();
        ui->savedListWidget->addItem(item);
        ui->savedListWidget->setItemWidget(item, itemWid);
    }
}

void SettingsDialog::updateSavedMarkingColors()
{
    ui->markingColorsListWidget->clear();
    QHash<QString, QColor> colors = helper->getMarkingsColor();
    QHashIterator<QString, QColor> i(colors);
    while (i.hasNext()) {
        i.next();
        QPixmap pix(20,20);
        pix.fill(i.value());
        DeleteableListItem *itemWid = new DeleteableListItem(i.key(), pix);
        connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onMarkingDelete(QString)));
        QListWidgetItem *item = new QListWidgetItem();
        ui->markingColorsListWidget->addItem(item);
        ui->markingColorsListWidget->setItemWidget(item, itemWid);
    }
}

void SettingsDialog::updateImportExportFuncs()
{
    ui->importExportListWidget->clear();
    QStringList list = helper->getImportExportFunctions();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new DeleteableListItem(list.at(i));
        connect(itemWid, SIGNAL(itemDeleted(QString)), this, SLOT(onImportExportFuncDeletes(QString)));
        QListWidgetItem *item = new QListWidgetItem();
        ui->importExportListWidget->addItem(item);
        ui->importExportListWidget->setItemWidget(item, itemWid);
    }
}

void SettingsDialog::onImportExportFuncDeletes(const QString &name)
{
    helper->removeImportExportFunctions(name);
}

void SettingsDialog::onResetImportExportFuncs()
{
    helper->resetImportExportFuncs();
}

void SettingsDialog::onDoubleClickImportExportFuncs(QListWidgetItem *item)
{
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->importExportListWidget->itemWidget(item);

    QString name = itemWid->getName();


    QuickViewItemConfig *itemConfig = new QuickViewItemConfig(helper, this);
    itemConfig->setWayBoxVisible(false);
    itemConfig->setFormatVisible(false);
    TransformAbstract *ta = helper->getImportExportFunction(name);

    if (ta != 0) {

        ta = tManager->loadTransformFromConf(ta->getConfiguration());
        if (ta != 0) {
            itemConfig->setTransform(ta);
            itemConfig->setName(name);
            int ret = itemConfig->exec();
            if (ret == QDialog::Accepted) {
                delete ta;
                ta = itemConfig->getTransform();
                QString newName = itemConfig->getName();

                helper->removeImportExportFunctions(name);
                helper->addImportExportFunctions(newName, ta);
            } else {
                delete ta;
            }

        }
    }
    delete itemConfig;
}

void SettingsDialog::onAddImportExportFuncs()
{
    QuickViewItemConfig *itemConfig = new QuickViewItemConfig(helper, this);
    itemConfig->setWayBoxVisible(false);
    itemConfig->setFormatVisible(false);
    int ret = itemConfig->exec();
    if (ret == QDialog::Accepted) {
        TransformAbstract *ta = itemConfig->getTransform();
        if (ta != 0) {
            helper->addImportExportFunctions(itemConfig->getName(),ta);
        }
    }

    delete itemConfig;
}

void SettingsDialog::onPluginClicked(QModelIndex index)
{
    QString name = index.data().toString();

    if (stackedList.contains(name)) {
        ui->pluginsStackedWidget->setCurrentIndex(stackedList.value(name));
    } else {
        emit error(tr("Cannot find the item %1 in stackedList T_T").arg(name), LOGID);
    }
}


void SettingsDialog::onSavedClicked(QListWidgetItem * item)
{
    DeleteableListItem *itemWid = (DeleteableListItem *)ui->savedListWidget->itemWidget(item);

    QString name = itemWid->getName();

    QHash<QString, QString> list = tManager->getSavedConfs();
    if (list.contains(name))
        ui->savedDescriptLabel->setText(tManager->getSavedConfs().value(name));
    else
        helper->getLogger()->logError(tr("Saved conf \"%1\" not found").arg(name),LOGID);
}

void SettingsDialog::onDeleteSaved(const QString &name)
{
    tManager->unregisterChainConf(name);
    ui->savedDescriptLabel->clear();
}

void SettingsDialog::onMarkingDelete(const QString &name)
{
    helper->removeMarkingColor(name);
}

void SettingsDialog::onResetMarkings()
{
    helper->resetMarkings();
}

void SettingsDialog::onServerPortChanged(int port)
{
    helper->setDefaultServerPort(port);
}

void SettingsDialog::onServerDecodeChanged(bool val)
{
    helper->setDefaultServerDecode(val);
}

void SettingsDialog::onServerEncodeChanged(bool val)
{
    helper->setDefaultServerEncode(val);
}

void SettingsDialog::onServerSeparatorChanged(char c)
{
    helper->setDefaultServerSeparator(c);
}

void SettingsDialog::onServerPipeNameChanged(QString name)
{
    helper->setDefaultServerPipeName(name);
}

