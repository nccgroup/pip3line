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
#include <QString>
#include <QMultiHash>
#include <QSettings>
#include <QColor>
#include <QSet>
#include <QMultiMap>
#include <QMutex>
#include <QMenu>
#include "sources/bytesourceabstract.h"
#include <transformabstract.h>
#include "transformsgui.h"
#include "textinputdialog.h"
#include "../tools/centralprocessor.h"

namespace GuiStyles {
    static const QString LineEditError = "QLineEdit { background-color: #FFB1B2; }";
    static const QString ComboBoxError = "QComboBox { color : red; }";
    static const QString LineEditWarning = "";
    static const QString LineEditMessage = "";
}

class TransformsGui;

class GuiHelper : public QObject
{
        Q_OBJECT
    public:
        static const QString UTF8_STRING_ACTION;
        static const QString SEND_TO_NEW_TAB_ACTION;
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

        TextInputDialog *getNameDialog(QWidget *parent, const QString &defaultvalue, const QString &title = QString());
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

        void updateCopyContextMenu(QMenu *copyMenu);
        void updateLoadContextMenu(QMenu *loadMenu);
        void loadAction(QString action, ByteSourceAbstract * byteSource);
        void copyAction(QString action, QByteArray value);
        void saveToFileAction(QByteArray value, QWidget *parent = 0);

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

        int getDefaultOffsetBase() const;
        void setDefaultOffsetBase(int val);



    public slots:

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
        Q_DISABLE_COPY(GuiHelper)
        static const QString SETTINGS_QUICKVIEWS;
        static const QString SETTINGS_FILTER_BLACKLIST;
        static const QString SETTINGS_MARKINGS_COLORS;
        static const QString SETTINGS_EXPORT_IMPORT_FUNC;
        static const QString SETTINGS_OFFSET_BASE;
        static const QString LOGID;
        static const QString NEW_BYTE_ACTION;
        static const int DEFAULT_OFFSET_BASE;
        bool eventFilter(QObject *o, QEvent *e);
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
        int offsetDefaultBase;
};

#endif // GUIHELPER_H
