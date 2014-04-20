/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QStandardItem>
#include <QStandardItemModel>
#include <QNetworkAccessManager>
#include <QApplication>
#include <QMutexLocker>
#include <QDebug>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <transformabstract.h>
#include <QXmlStreamWriter>
#include "guihelper.h"
#include "tabs/tababstract.h"
#include "sources/bytesourceabstract.h"
#include "newbytedialog.h"
#include "loggerwidget.h"
#include "textinputdialog.h"
#include <transformmgmt.h>
#include <threadedprocessor.h>

using namespace Pip3lineConst;

const QString GuiHelper::SETTINGS_QUICKVIEWS = "QuickView";
const QString GuiHelper::SETTINGS_FILTER_BLACKLIST = "FilterBlacklist";
const QString GuiHelper::SETTINGS_MARKINGS_COLORS = "MarkingsColors";
const QString GuiHelper::SETTINGS_EXPORT_IMPORT_FUNC = "ExportImportFunctions";
const QString GuiHelper::SETTINGS_OFFSET_BASE = "OffsetBase";
const QString GuiHelper::UTF8_STRING_ACTION = "UTF-8";
const QString GuiHelper::LOGID = "GuiHelper";
const QString GuiHelper::NEW_BYTE_ACTION = "New Byte(s)";
const QString GuiHelper::SEND_TO_NEW_TAB_ACTION = "New tab";
const int GuiHelper::DEFAULT_OFFSET_BASE = 16;

GuiHelper::GuiHelper(TransformMgmt *ntransformFactory, QNetworkAccessManager *nmanager, LoggerWidget *nlogger)
{
    transformFactory = ntransformFactory;
    networkManager = nmanager;
    logger = nlogger;
    centralTransProc = NULL;
    settings = transformFactory->getSettingsObj();
    bool ok = false;
    defaultServerPort = settings->value(SETTINGS_SERVER_PORT,DEFAULT_PORT).toInt(&ok);
    if (!ok || defaultServerPort < 1)
        defaultServerPort = DEFAULT_PORT;

    QByteArray temp = settings->value(SETTINGS_SERVER_SEPARATOR,QByteArray()).toByteArray();
    temp = QByteArray::fromHex(temp);
    if (temp.isEmpty())
        defaultServerSeparator = DEFAULT_BLOCK_SEPARATOR;
    else
        defaultServerSeparator = temp.at(0);

    defaultServerDecode = settings->value(SETTINGS_SERVER_DECODE,false).toBool();
    defaultServerEncode = settings->value(SETTINGS_SERVER_ENCODE, false).toBool();
    defaultServerPipeName = settings->value(SETTINGS_SERVER_PIPE_NAME, DEFAULT_PIPE_MASS).toString();

    offsetDefaultBase = settings->value(SETTINGS_OFFSET_BASE, DEFAULT_OFFSET_BASE).toInt(&ok);
    if (!ok || (offsetDefaultBase != 8 && offsetDefaultBase != 10 && offsetDefaultBase != 16)) {
        offsetDefaultBase = DEFAULT_OFFSET_BASE;
    }

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
    centralTransProc = new(std::nothrow) ThreadedProcessor();
    if (centralTransProc == NULL) {
        qFatal("Cannot allocate memory for action CentralProcessor X{");
    }
}

GuiHelper::~GuiHelper()
{
    delete centralTransProc;
    centralTransProc = NULL;
    saveImportExportFunctions();
    deleteImportExportFuncs();
    delete settings;
    logger = NULL;
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

void GuiHelper::sendNewSelection(const QByteArray &selection)
{
    emit newSelection(selection);
}

void GuiHelper::sendToNewTab(const QByteArray &initialValue)
{
    emit newTabRequested(initialValue);
}

void GuiHelper::addTab(TabAbstract *tab)
{
    tabs.insert(tab);
    connect(tab, SIGNAL(destroyed()), this, SLOT(onTabDeleted()), Qt::UniqueConnection);
    updateSortedTabs();
    emit tabsUpdated();
}

void GuiHelper::removeTab(TabAbstract *tab)
{
    tabs.remove(tab);
    updateSortedTabs();
    emit tabsUpdated();
}

QList<TabAbstract *> GuiHelper::getTabs()
{
    return sortedTabs.values();
}

void GuiHelper::onTabDeleted()
{
    TabAbstract * tg = static_cast<TabAbstract *>(sender());
    if (tg == NULL) {
        logger->logError(tr("NULL Deleted Tab"), LOGID);
    } else if (!tabs.remove(tg)) {
        logger->logError(tr("Deleted Tab not found"), LOGID);
    } else {
        updateSortedTabs();
        emit tabsUpdated();
    }
}

void GuiHelper::raisePip3lineWindow()
{
    emit raiseWindowRequest();
}

void GuiHelper::updateSortedTabs()
{
    sortedTabs.clear();
    QSetIterator<TabAbstract *> i(tabs);
    while (i.hasNext()) {
     TabAbstract * tg = i.next();
     sortedTabs.insertMulti(tg->getName(), tg);
    }
}

TextInputDialog *GuiHelper::getNameDialog(QWidget *parent,const QString &defaultvalue, const QString &title)
{
    TextInputDialog *nameDialog; // yes this has to be cleaned at the upper layer.

    nameDialog = new(std::nothrow) TextInputDialog(parent);
    if (nameDialog != NULL) {

        nameDialog->setModal(true);
        if (title.isEmpty())
            nameDialog->setWindowTitle(tr("Name"));
        else
            nameDialog->setWindowTitle(title);
        nameDialog->setText(defaultvalue);
    } else {
        qFatal("Could not instanciate textInputDialog X{");
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
    QStringList list = importExportFunctions.keys();
    qSort(list);
    return list;
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
ThreadedProcessor *GuiHelper::getCentralTransProc() const
{
    return centralTransProc;
}

QSet<QString> GuiHelper::getTypesBlacklist() const
{
    return typesBlacklist;
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
    settings->setValue(SETTINGS_SERVER_PORT, defaultServerPort);
}

int GuiHelper::getDefaultPort() const
{
    return defaultServerPort;
}

void GuiHelper::setDefaultServerPipeName(const QString &local)
{
    defaultServerPipeName = local;
    settings->setValue(SETTINGS_SERVER_PIPE_NAME, defaultServerPipeName);
}

QString GuiHelper::getDefaultServerPipeName() const
{
    return defaultServerPipeName;
}

void GuiHelper::setDefaultServerDecode(bool val)
{
    defaultServerDecode = val;
    settings->setValue(SETTINGS_SERVER_DECODE, defaultServerDecode);
}

bool GuiHelper::getDefaultServerDecode() const
{
    return defaultServerDecode;
}

void GuiHelper::setDefaultServerEncode(bool val)
{
    if (defaultServerEncode != val) {
        defaultServerEncode = val;
        settings->setValue(SETTINGS_SERVER_ENCODE, defaultServerEncode);
    }
}

bool GuiHelper::getDefaultServerEncode() const
{
    return defaultServerEncode;
}

void GuiHelper::setDefaultServerSeparator(char sep)
{
    if (defaultServerSeparator != sep) {
        defaultServerSeparator = sep;

    }
}

char GuiHelper::getDefaultServerSeparator() const
{
    return defaultServerSeparator;
}

int GuiHelper::getDefaultOffsetBase() const
{
    return offsetDefaultBase;
}

void GuiHelper::setDefaultOffsetBase(int val)
{
    if (offsetDefaultBase != val && (offsetDefaultBase == 8 || offsetDefaultBase == 10 || offsetDefaultBase == 16)) {
        offsetDefaultBase = val;
        settings->setValue(SETTINGS_OFFSET_BASE,  offsetDefaultBase);
    }
}

void GuiHelper::goIntoHidding()
{
    emit appGoesIntoHidding();
}

void GuiHelper::isRising()
{
    emit appIsRising();
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

void GuiHelper::updateCopyContextMenu(QMenu *copyMenu)
{
    QAction * action = NULL;
    copyMenu->clear();

    action = new(std::nothrow) QAction(GuiHelper::UTF8_STRING_ACTION, copyMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus UTF8 X{");
        return;
    }
    copyMenu->addAction(action);

    QStringList list = getImportExportFunctions();

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), copyMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus copyMenu X{");
            return;
        }
        copyMenu->addAction(action);
    }
}

void GuiHelper::updateLoadContextMenu(QMenu *loadMenu)
{
    QAction * action = NULL;
    loadMenu->clear();
    action = new(std::nothrow) QAction(GuiHelper::UTF8_STRING_ACTION, loadMenu);
    if (action == NULL) {
        qFatal("Cannot allocate memory for action updateImportExportMenus loadMenu UTF8 X{");
        return;
    }
    loadMenu->addAction(action);

    QStringList list = getImportExportFunctions();

    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), loadMenu);
        if (action == NULL) {
            qFatal("Cannot allocate memory for action updateImportExportMenus loadMenu user's X{");
            return;
        }
        loadMenu->addAction(action);
    }
}

void GuiHelper::loadAction(QString action, ByteSourceAbstract *byteSource)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString input = clipboard->text();
    if (action == NEW_BYTE_ACTION) {
        NewByteDialog *dialog = new(std::nothrow) NewByteDialog(this);
        if (dialog == NULL) {
            qFatal("Cannot allocate memory for action NewByteDialog X{");
            return;
        }
        dialog->setModal(true);
        int ret = dialog->exec();
        if (ret == QDialog::Accepted) {
            byteSource->setData(QByteArray(dialog->byteCount(),dialog->getChar()));
        }
        delete dialog;

    } else if (action == GuiHelper::UTF8_STRING_ACTION) {
        byteSource->setData(input.toUtf8());
    } else {
        TransformAbstract *ta  = getImportExportFunction(action);
        if (ta != NULL) {
            ta->setWay(TransformAbstract::OUTBOUND);
            byteSource->setData(ta->transform(input.toUtf8()));
        }
    }
}

void GuiHelper::copyAction(QString action, QByteArray value)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (action == GuiHelper::UTF8_STRING_ACTION) {
        clipboard->setText(QString::fromUtf8(value));
    } else {
        TransformAbstract *ta  = getImportExportFunction(action);
        if (ta != NULL) {
            ta->setWay(TransformAbstract::INBOUND);
            clipboard->setText(ta->transform(value));
        }
    }
}

void GuiHelper::saveToFileAction(QByteArray value, QWidget *parent)
{
    QString fileName = QFileDialog::getSaveFileName(parent,tr("Choose a file to save to"));
    if (!fileName.isEmpty()) {

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QString mess = tr("Failed to open the file for writing:\n %1").arg(file.errorString());
            logger->logError(mess);
            QMessageBox::critical(parent,tr("Error"), mess,QMessageBox::Ok);
            return;
        }
        file.write(value);
        file.close();
    }
}

const QString GuiHelper::getXMLfromRes(const QString &res)
{
    QFile file(res);
    file.open(QIODevice::ReadOnly);
    return QString::fromUtf8(file.readAll());
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
