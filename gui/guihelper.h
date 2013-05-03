/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef GUIHELPER_H
#define GUIHELPER_H

#include "loggerwidget.h"
#include <transformmgmt.h>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QObject>
#include <QHash>
#include <QDialog>
#include <QComboBox>
#include <QMultiHash>
#include <QSettings>
#include <QColor>
#include <QSet>
#include <QMultiMap>
#include <QMutex>
#include <transformabstract.h>
#include "transformsgui.h"
#include "../tools/centralprocessor.h"
#include "ui_tabname.h"

class TransformsGui;

class NameDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit NameDialog(QWidget *parent = 0);
        ~NameDialog();
        void setDefaultValue(const QString &value);
        QString getName() const;
    private:
        Ui::TabName tabNameUI;
};

class GuiHelper : public QObject
{
        Q_OBJECT
    public:
        static const QString ACTION_UTF8_STRING;
        explicit GuiHelper( TransformMgmt *transformFactory, QNetworkAccessManager *networkManager, LoggerWidget *logger);
        ~GuiHelper();
        LoggerWidget *getLogger();
        TransformMgmt *getTransformFactory();
        QNetworkAccessManager *getNetworkManager();

        void processTransform(TransformRequest * request);

        void sendNewSelection(const QByteArray &selection);

        void sendToNewTab(const QByteArray &initialValue = QByteArray());
        void addTab(TransformsGui *tab);
        void removeTab(TransformsGui * tab);
        QList<TransformsGui *> getTabs();

        NameDialog *getNameDialog(QWidget *parent, const QString &defaultvalue, const QString &title = QString());
        void buildTransformComboBox(QComboBox *box, const QString &defaultSelected = QString(), bool applyFilter = false);
        void buildFilterComboBox(QComboBox *box);

        QStringList getDefaultQuickViews();
        QStringList getQuickViewConf();
        void saveQuickViewConf(QStringList conf);

        QHash<QString, QColor> getDefaultMarkingsColor();
        QHash<QString, QColor> getMarkingsColor();
        void resetMarkings();
        void addNewMarkingColor(const QString &name, const QColor &color);
        void removeMarkingColor(const QString &name);


        QStringList getImportExportFunctions();
        TransformAbstract * getImportExportFunction(const QString &name);
        void resetImportExportFuncs();
        void addImportExportFunctions(const QString &name, TransformAbstract *ta);
        void removeImportExportFunctions(const QString &name);

        void setDefaultServerPort(int port);
        int getDefaultPort() const;
        void setDefaultServerPipeName(const QString &local);
        QString getDefaultServerPipeName() const;
        void setDefaultServerDecode(bool val);
        bool getDefaultServerDecode() const;
        void setDefaultServerEncode(bool val);
        bool getDefaultServerEncode() const;
        void setDefaultServerSeparator(char sep);
        char getDefaultServerSeparator() const;

        bool eventFilter(QObject *o, QEvent *e);

    signals:
        void newSelection(QByteArray selection);
        void newTabRequested(QByteArray initialValue);
        void filterChanged();
        void markingsUpdated();
        void importExportUpdated();
        void tabsUpdated();
    private slots:
        void onFilterChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
        void onTabDeleted();
    private:
        static const QString SETTINGS_QUICKVIEWS;
        static const QString SETTINGS_FILTER_BLACKLIST;
        static const QString SETTINGS_MARKINGS_COLORS;
        static const QString SETTINGS_EXPORT_IMPORT_FUNC;
        static const QString LOGID;
        inline void updateSortedTabs();
        void loadImportExportFunctions();
        void saveImportExportFunctions();
        const QString getXMLfromRes(const QString &res);
        void saveMarkingsColor();
        void deleteImportExportFuncs();
        TransformMgmt *transformFactory;
        QNetworkAccessManager *networkManager;
        LoggerWidget *logger;
        QSettings *settings;
        QSet<TransformsGui *> tabs;
        QMultiMap<QString, TransformsGui *> sortedTabs;
        int defaultServerPort;
        QString defaultServerIp;
        QString defaultServerPipeName;
        bool defaultServerDecode;
        bool defaultServerEncode;
        char defaultServerSeparator;
        QSet<QString> typesBlacklist;
        QHash<QString, QColor> markingColors;
        QHash<QString , TransformAbstract *> importExportFunctions;
        CentralProcessor centralTransProc;
};

#endif // GUIHELPER_H
