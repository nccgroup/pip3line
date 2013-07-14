/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMMGMT_H
#define TRANSFORMMGMT_H

#include "libtransform_global.h"

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QMutex>
#include <QStringList>
#include <QList>
#include <QtPlugin>
#include <QPluginLoader>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QRegExp>
#include <QSettings>
#include <QSemaphore>
#include <QSet>
#include "transformabstract.h"
#include "transformfactoryplugininterface.h"
#include "pip3linecallback.h"

#include "transformchain.h"


class LIBTRANSFORMSHARED_EXPORT TransformMgmt : public QObject
{
        Q_OBJECT
    public:
        static const QString CONF_FILE;
        static const QString APP_PLUGIN_DIRECTORY;
        static const int MAX_TYPE_NAME_LENGTH;
        static const QString SETTINGS_SAVED_CONF;

        explicit TransformMgmt();
        ~TransformMgmt();
        bool initialize(const QString &baseDirectory);


        const QStringList getTransformsList(QString typeName);
        const QStringList getTypesList();

        QHash<QString, TransformFactoryPluginInterface *> getPlugins();
        QStringList getPluginsDirectory();
        static QString getHomeDirectory();

        TransformAbstract * getTransform(QString name); // the central function
        TransformAbstract * loadTransformFromConf(const QHash<QString, QString> confEle);
        TransformAbstract * loadComposedTransformFromXML(QXmlStreamReader *streamReader);
        TransformAbstract * loadComposedTransformFromXML(const QString &conf);

        TransformChain loadConfFromXML(const QString &conf);
        TransformChain loadConfFromXML(QXmlStreamReader *streamReader);
        TransformChain loadConfFromFile(const QString &fileName);

        bool saveConfToXML(const TransformChain &transfList,  QXmlStreamWriter *stream);
        bool saveConfToFile(const QString &fileName, const TransformChain &transfList);

        bool registerChainConf(const TransformChain &transfChain, bool persistent = false);
        bool unregisterChainConf(const QString &name);
        void setPersistance(const QString &name, bool persistent);
        TransformChain loadChainFromSaved(const QString &name);

        QHash<QString, QString> getSavedConfs();

        QSettings *getSettingsObj();

    Q_SIGNALS:
        void error(const QString,const QString);
        void warning(const QString,const QString);
        void status(const QString,const QString);
        void transformsUpdated();
        void savedUpdated();
    private Q_SLOTS:
        void logError(const QString mess, const QString source = LOG_ID);
        void logWarning(const QString mess, const QString source = LOG_ID);
        void logStatus(const QString mess, const QString source = LOG_ID);
        bool loadTransforms(bool verbose = false);
        void OnTransformDelete();
    private:
        Q_DISABLE_COPY(TransformMgmt)
        static const QRegExp TAG_NAME_REGEXP;
        static const int MAX_NESTING;
        static const QString LOG_ID;
        inline bool isValidAttributeName(QString name);
        void registerPlugin(TransformFactoryPluginInterface * plugin);

        void unloadTransforms();
        bool loadPlugins();
        void unloadPlugins();
        void saveInstance(TransformAbstract * ta);

        QHash<QString, QStringList> transformTypesList;
        QHash<QString, TransformFactoryPluginInterface *> transformNameList;
        QHash<TransformFactoryPluginInterface *, Pip3lineCallback *> callbackList;
        QMutex listLocker;
        QHash<QString, TransformFactoryPluginInterface *> pluginsList;
        QList<QPluginLoader *> pluginLibs;
        QStringList typesList;
        QString id;
        QStringList pluginsDirectories;
        QHash<QString, QString> savedConf;
        QSettings * settings;
        QString fileConf;
        QSemaphore cycleSem;
        QSet<TransformAbstract *> transformInstances;
};

#endif // TRANSFORMMGMT_H
