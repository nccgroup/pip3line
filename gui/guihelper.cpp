/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QStandardItem>
#include <QStandardItemModel>
#include <QMutexLocker>
#include <QDebug>
#include "guihelper.h"
using namespace Pip3lineConst;

const QString GuiHelper::SETTINGS_QUICKVIEWS = "QuickView";
const QString GuiHelper::SETTINGS_FILTER_BLACKLIST = "FilterBlacklist";
const QString GuiHelper::SETTINGS_MARKINGS_COLORS = "MarkingsColors";
const QString GuiHelper::SETTINGS_EXPORT_IMPORT_FUNC = "ExportImportFunctions";
const QString GuiHelper::ACTION_UTF8_STRING = "UTF-8";
const QString GuiHelper::LOGID = "GuiHelper";

GuiHelper::GuiHelper(TransformMgmt *ntransformFactory, QNetworkAccessManager *nmanager, LoggerWidget *nlogger)
{
    transformFactory = ntransformFactory;
    networkManager = nmanager;
    logger = nlogger;
    settings = transformFactory->getSettingsObj();
    bool ok = false;
    defaultServerPort = settings->value(SETTINGS_SERVER_PORT,DEFAULT_PORT).toInt(&ok);
    if (defaultServerPort < 1)
        defaultServerPort = DEFAULT_PORT;

    int temp = settings->value(SETTINGS_SERVER_SEPARATOR,DEFAULT_BLOCK_SEPARATOR).toInt(&ok);
    if (!ok || temp < 0x00 || temp > 0xFF)
        defaultServerSeparator = DEFAULT_BLOCK_SEPARATOR;
    else
        defaultServerSeparator = (char) temp;

    defaultServerDecode = settings->value(SETTINGS_SERVER_DECODE,false).toBool();
    defaultServerEncode = settings->value(SETTINGS_SERVER_ENCODE, false).toBool();
    defaultServerPipeName = settings->value(SETTINGS_SERVER_PIPE_NAME, DEFAULT_PIPE_MASS).toString();

    QStringList blacklist = settings->value(SETTINGS_FILTER_BLACKLIST, QStringList()).toStringList();
    typesBlacklist = blacklist.toSet();

    QHash<QString, QVariant> hash = settings->value(SETTINGS_MARKINGS_COLORS).toHash();
    if (hash.isEmpty())
        markingColors = getDefaultMarkingsColor();
    else {
        QHashIterator<QString, QVariant> i(hash);
        while (i.hasNext()) {
            i.next();
            QColor color(i.value().toUInt(&ok));
            if (!ok) {
                logger->logError(tr("Invalid color for marking from persistant conf"), LOGID);
                continue;
            }
            markingColors.insert(i.key(),color);
        }
    }

    loadImportExportFunctions();
    centralTransProc.start();
}

GuiHelper::~GuiHelper()
{
    centralTransProc.stop();
    saveImportExportFunctions();
    deleteImportExportFuncs();
    delete settings;
}

LoggerWidget *GuiHelper::getLogger()
{
    return logger;
}

TransformMgmt *GuiHelper::getTransformFactory()
{
    return transformFactory;
}

QNetworkAccessManager *GuiHelper::getNetworkManager()
{
    return networkManager;
}

void GuiHelper::processTransform(TransformRequest *request)
{
    centralTransProc.addToProcessingQueue(request);
}

void GuiHelper::sendNewSelection(const QByteArray &selection)
{
    emit newSelection(selection);
}

void GuiHelper::sendToNewTab(const QByteArray &initialValue)
{
    emit newTabRequested(initialValue);
}

void GuiHelper::addTab(TransformsGui * tab)
{
    tabs.insert(tab);
    connect(tab, SIGNAL(destroyed()), this, SLOT(onTabDeleted()), Qt::UniqueConnection);
    updateSortedTabs();
    emit tabsUpdated();
}

void GuiHelper::removeTab(TransformsGui * tab)
{
    tabs.remove(tab);
    updateSortedTabs();
    emit tabsUpdated();
}

QList<TransformsGui *> GuiHelper::getTabs()
{
    return sortedTabs.values();
}

void GuiHelper::onTabDeleted()
{
    TransformsGui * tg = (TransformsGui *)sender();
    if (!tabs.remove(tg)) {
        logger->logError(tr("Deleted Tab not found"), LOGID);
    } else {
        updateSortedTabs();
        emit tabsUpdated();
    }
}

void GuiHelper::updateSortedTabs()
{
    sortedTabs.clear();
    QSetIterator<TransformsGui *> i(tabs);
    while (i.hasNext()) {
     TransformsGui * tg = i.next();
     sortedTabs.insertMulti(tg->getName(), tg);
    }
}

NameDialog *GuiHelper::getNameDialog(QWidget *parent,const QString &defaultvalue, const QString &title)
{
    NameDialog *nameDialog; // yes this has to be cleaned at the upper layer.

    nameDialog = new(std::nothrow) NameDialog(parent);
    if (nameDialog != NULL) {

        nameDialog->setModal(true);
        if (title.isEmpty())
            nameDialog->setWindowTitle(tr("Name"));
        else
            nameDialog->setWindowTitle(title);
        nameDialog->setDefaultValue(defaultvalue);
    } else {
        qFatal("Could not instanciate nameDialog X{");
    }

    return nameDialog;
}

void GuiHelper::buildTransformComboBox(QComboBox *box, const QString &defaultSelected, bool applyFilter)
{
    // creating transform combo box
    // Adding the first element (unselectable)
    QString tempLabel = tr("Choose Transformation");
    box->addItem(tempLabel);
    int row = 0;
    int defaultIndex = -1;

    // Set the flag of the item within the combobox model
    QStandardItem * item = qobject_cast<QStandardItemModel *>(box->model())->item(row);
    item->setEnabled( false );
    QStringList typesList = transformFactory->getTypesList();
    for (int i = 0; i < typesList.size(); i++) {
        if (applyFilter && typesBlacklist.contains(typesList.at(i)))
            continue;
        QStringList list = transformFactory->getTransformsList(typesList.at(i));
        if (list.isEmpty())
            continue;
        qSort(list);
        tempLabel = typesList.at(i);
        row = box->model()->rowCount();
        box->addItem(tempLabel);

        // Set the flag of the item within the combobox model
        QStandardItem * item = qobject_cast<QStandardItemModel *>(box->model())->item(row);
        item->setEnabled( false );
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackground(Qt::darkGray);
        item->setForeground(Qt::white);
        //adding the transform for this category
        for (int j = 0; j < list.size(); j++) {
            if (list.at(j) == defaultSelected) {
                defaultIndex = box->model()->rowCount();
            }
            box->addItem(list.at(j));
        }
    }

    if (defaultIndex != -1)
        box->setCurrentIndex(defaultIndex);
    else if (!defaultSelected.isEmpty()){
        box->insertItem(1,defaultSelected);
        box->setCurrentIndex(1);
    }
}

void GuiHelper::buildFilterComboBox(QComboBox *box)
{
    QStringList typesList = transformFactory->getTypesList();
    QStandardItemModel *model = new(std::nothrow) QStandardItemModel(typesList.size(), 1);
    if (model == NULL ) {
        qFatal("Cannot allocate memory for QStandardItemModel X{");
        return;
    }
    QStandardItem* item = new(std::nothrow) QStandardItem(tr("Transformations filter"));
    if (item == NULL) {
        qFatal("Cannot allocate memory for QStandardItem 1 X{");
        delete model;
        return;
    }

    item->setEnabled(false);
    model->setItem(0, 0, item);
    for (int i = 0; i < typesList.size(); ++i)
    {
        item = new(std::nothrow) QStandardItem(typesList.at(i));
        if (item != NULL) {
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setData( (typesBlacklist.contains(typesList.at(i)) ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
            model->setItem(i + 1, 0, item);
        } else {
          qFatal("Cannot allocate memory for QStandardItem 2 X{");
        }
    }
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onFilterChanged(QModelIndex,QModelIndex)));
    box->setModel(model);
    box->setCurrentIndex(0);
}

QStringList GuiHelper::getDefaultQuickViews()
{
    QStringList list;
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int8lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int16lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int32lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int64lesign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/int64leunsign.xml"));
    list.append(getXMLfromRes(":/xmlconfs/defaultsxml/utf16le.xml"));
    return list;
}

QStringList GuiHelper::getQuickViewConf()
{
    QStringList list = settings->value(SETTINGS_QUICKVIEWS, QStringList()).toStringList();
    if (list.isEmpty()) {
        list = getDefaultQuickViews();
    }
    return list;
}

void GuiHelper::saveQuickViewConf(QStringList conf)
{
    settings->setValue(SETTINGS_QUICKVIEWS, conf);
}

QHash<QString, QColor> GuiHelper::getDefaultMarkingsColor()
{
    QHash<QString, QColor> defaultColors;
    defaultColors.insert("Data",QColor(11449599));
    defaultColors.insert("Size",QColor(16755616));
    defaultColors.insert("Type",QColor(9043881));
    return defaultColors;
}

QHash<QString, QColor> GuiHelper::getMarkingsColor()
{
    return markingColors;
}

void GuiHelper::resetMarkings()
{
    markingColors.clear();
    saveMarkingsColor();
    markingColors = getDefaultMarkingsColor();
    emit markingsUpdated();
}

void GuiHelper::addNewMarkingColor(const QString &name, const QColor &color)
{
    markingColors.insert(name, color);
    saveMarkingsColor();
    emit markingsUpdated();
}

void GuiHelper::removeMarkingColor(const QString &name)
{
    if (markingColors.contains(name)) {
        markingColors.remove(name);
        saveMarkingsColor();
        emit markingsUpdated();
    }
}


void GuiHelper::loadImportExportFunctions()
{
    QStringList list = settings->value(SETTINGS_EXPORT_IMPORT_FUNC, QStringList()).toStringList();
    if (list.isEmpty()) {
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/base64decode.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/hexadecimal.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/escapedhexadecimal.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/escapedhexadecimalmixed.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/cstylearray.xml"));
        list.append(getXMLfromRes(":/xmlconfs/defaultsxml/urlencoded.xml"));
    }
    TransformAbstract * ta = NULL;
    for (int i = 0; i < list.size(); i++) {
        TransformChain talist = transformFactory->loadConfFromXML(list.at(i));
        if (talist.isEmpty()) {
            logger->logError(tr("Could not load the configuration for Export/Import (skipping)\n%1").arg(list.at(i)), LOGID);
            continue;
        }
        else {
            ta = talist.takeFirst();
            connect(ta, SIGNAL(error(QString,QString)), logger, SLOT(logError(QString,QString)));
            connect(ta, SIGNAL(warning(QString,QString)), logger, SLOT(logWarning(QString,QString)));
            if (talist.size() > 0) {
                logger->logError(tr("Configuration loaded multiple transform, clearing the remaining ones."), LOGID);
                while (!talist.isEmpty()) {
                    delete talist.takeLast();
                }
            }
            importExportFunctions.insert(talist.getName(), ta);
        }

    }

}

void GuiHelper::saveImportExportFunctions()
{
    QHashIterator<QString, TransformAbstract *> i(importExportFunctions);
    QStringList savedConf;
    while (i.hasNext()) {
         i.next();
         QString conf;
         TransformChain list;
         list.append(i.value());
         list.setName(i.key());
         QXmlStreamWriter writer(&conf);
         transformFactory->saveConfToXML(list,&writer);
         savedConf.append(conf);
     }
    settings->setValue(SETTINGS_EXPORT_IMPORT_FUNC, savedConf);
}

QStringList GuiHelper::getImportExportFunctions()
{
    return importExportFunctions.keys();
}

TransformAbstract *GuiHelper::getImportExportFunction(const QString &name)
{

    if (name.isEmpty()) {
        logger->logError(tr("Empty export/import function name"));
        return NULL;
    }

    if (importExportFunctions.contains(name)) {
        return importExportFunctions.value(name);
    }

    logger->logError(tr("Unkown import/export function name: \"%1\"").arg(name));
    return NULL;
}

void GuiHelper::resetImportExportFuncs()
{
    deleteImportExportFuncs();
    saveImportExportFunctions();
    loadImportExportFunctions();
    emit importExportUpdated();
}

void GuiHelper::addImportExportFunctions(const QString &name, TransformAbstract * ta)
{
    if (name.isEmpty()) {
        logger->logError(tr("Empty export/import function name. Ignoring add request."));
        return;
    }

    if (ta == NULL) {
        logger->logError(tr("Empty export/import function \"%1\" is NULL. Ignoring add request.").arg(name));
        return;
    }

    if (importExportFunctions.contains(name)) { // overwriting by default
        TransformAbstract * oldta = importExportFunctions.take(name);
        delete oldta;
    }

    importExportFunctions.insert(name, ta);

    saveImportExportFunctions();

    emit importExportUpdated();
}

void GuiHelper::removeImportExportFunctions(const QString &name)
{
    if (name.isEmpty()) {
        logger->logError(tr("Empty export/import function name. Ignoring remove request."));
        return;
    }

    if (!importExportFunctions.contains(name)) {
        logger->logError(tr("Unknown export/import function \"%1\". Ignoring remove request.").arg(name));
        return;
    }

    TransformAbstract * ta = importExportFunctions.take(name);
    delete ta;

    saveImportExportFunctions();

    emit importExportUpdated();
}

void GuiHelper::deleteImportExportFuncs()
{
    if (!importExportFunctions.isEmpty()) {
        QHashIterator<QString , TransformAbstract *> i(importExportFunctions);
         while (i.hasNext()) {
             i.next();
             delete i.value();
         }

        importExportFunctions.clear();
    }
}

void GuiHelper::saveMarkingsColor()
{
    QHash<QString, QVariant> colors;
    QHashIterator<QString, QColor> i(markingColors);
    while (i.hasNext()) {
        i.next();
        colors.insert(i.key(),i.value().rgb());
    }
    settings->setValue(SETTINGS_MARKINGS_COLORS, colors);
}

void GuiHelper::setDefaultServerPort(int port)
{
    defaultServerPort = port;
}

int GuiHelper::getDefaultPort() const
{
    return defaultServerPort;
}

void GuiHelper::setDefaultServerPipeName(const QString &local)
{
    defaultServerPipeName = local;
}

QString GuiHelper::getDefaultServerPipeName() const
{
    return defaultServerPipeName;
}

void GuiHelper::setDefaultServerDecode(bool val)
{
    defaultServerDecode = val;
}

bool GuiHelper::getDefaultServerDecode() const
{
    return defaultServerDecode;
}

void GuiHelper::setDefaultServerEncode(bool val)
{
    defaultServerEncode = val;
}

bool GuiHelper::getDefaultServerEncode() const
{
    return defaultServerEncode;
}

void GuiHelper::setDefaultServerSeparator(char sep)
{
    defaultServerSeparator = sep;
}

char GuiHelper::getDefaultServerSeparator() const
{
    return defaultServerSeparator;
}

bool GuiHelper::eventFilter(QObject *o, QEvent *e)
{
    // Filtering out wheel event for comboboxes
    if ( e->type() == QEvent::Wheel && qobject_cast<QComboBox*>(o) )
    {
        e->ignore();
        return true;
    }
    return false;
}

const QString GuiHelper::getXMLfromRes(const QString &res)
{
    QFile file(res);
    file.open(QIODevice::ReadOnly);
    return QString::fromUtf8(file.readAll());
}

NameDialog::NameDialog(QWidget *parent) : QDialog(parent)
{
    tabNameUI.setupUi(this);
}

NameDialog::~NameDialog()
{
}

void NameDialog::setDefaultValue(const QString &value)
{
    tabNameUI.nameLineEdit->setText(value);
    tabNameUI.nameLineEdit->selectAll();
}

QString NameDialog::getName() const
{
    return tabNameUI.nameLineEdit->text();
}


void GuiHelper::onFilterChanged(const QModelIndex &topLeft, const QModelIndex &)
{
    QString item = topLeft.data().toString();
    Qt::CheckState state = (Qt::CheckState) topLeft.data(Qt::CheckStateRole).toInt();
    if (state == Qt::Unchecked) {
        typesBlacklist.insert(item);
    } else {
        typesBlacklist.remove(item);
    }
    settings->setValue(SETTINGS_FILTER_BLACKLIST, (QStringList)typesBlacklist.toList());
    emit filterChanged();
}
