/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "opensslplugin.h"
#include "opensslhashes.h"
#include <QTextStream>
#include <QLabel>
#include <QDebug>
#include "../../version.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>

const QStringList OpensslPlugin::blacklistHash = QStringList() << "ecdsa-with-SHA1" << "DSA" << "DSA-SHA" << "MD5" << "MD4" << "SHA1";
QMutex OpensslPlugin::hashListLocker;

extern "C" {
void static list_md_fn_OpenSSLHashes(const EVP_MD *m, const char */* unused */, const char */* unused */, void */* unused */)
{

    if (m) {
        QString name(EVP_MD_name(m));

        // blacklisting some messages digest returned by openssl list function.
        // If someone can explain to me why the list of hashes in here is different
        // from the one given by the openssl command line tool that would be great.
        // Not only it returns a different list, but the list itself is mostly duplicated ??
        // Blacklist also includes hashes already present by default in the Qt API

        if (OpensslPlugin::blacklistHash.contains(name)) {
            return;
        }
        if (!OpenSSLHashes::hashList.contains(name)) {
            qDebug() << name << " " << m << " " << EVP_MD_type(m);
            OpenSSLHashes::hashList.append(name);
        }
    }
}
}

OpensslPlugin::OpensslPlugin()
{
    hashListLocker.lock();
    if (OpenSSLHashes::hashList.isEmpty()) {
        OpenSSL_add_all_digests();
        EVP_MD_do_all_sorted(list_md_fn_OpenSSLHashes, 0);
    }
    hashListLocker.unlock();
    gui = 0;
}

OpensslPlugin::~OpensslPlugin()
{
    qDebug() << "Destroying Openssl Hashes Plugins" << this;
    hashListLocker.lock();
    OpenSSLHashes::hashList.clear();
    hashListLocker.unlock();
    EVP_cleanup();
    if (gui != 0) {
        delete gui;
    }
}

QString OpensslPlugin::pluginName() const
{
    return "Openssl Hashes";
}

TransformAbstract *OpensslPlugin::getTransform(QString name)
{
    if (OpenSSLHashes::hashList.contains(name)) {
        return new OpenSSLHashes(name);
    } else if (name == OpenSSLHashes::id) {
        return new OpenSSLHashes();
    }
    return 0;
}

const QStringList OpensslPlugin::getTransformList(QString type)
{
    QStringList ret;
    if (type == DEFAULT_TYPE_HASHES) {
        ret.append(OpenSSLHashes::hashList);
    }
    return ret;
}

const QStringList OpensslPlugin::getTypesList()
{
    return QStringList() << DEFAULT_TYPE_HASHES;
}

QWidget *OpensslPlugin::getConfGui(QWidget * /* parent */)
{
    if (gui == 0) {
        QString info;
        info.append(QString("<p>Plugin compiled against %1<br>").arg(OPENSSL_VERSION_TEXT));
        info.append("Plugin currently running with: <ul>");
        info.append(QString("<li>Version: %1</li>").arg(SSLeay_version(SSLEAY_VERSION)));
        info.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_PLATFORM)));
        info.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_CFLAGS)));
        info.append(QString("<li>%1</li>").arg(SSLeay_version(SSLEAY_BUILT_ON)));

        info.append("</ul></p>");
        QLabel * label = new QLabel(info);
        label->setWordWrap(true);
        gui = label;
        connect(gui,SIGNAL(destroyed()), SLOT(onGuiDelete()));
    }
    return gui;
}

QString OpensslPlugin::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

int OpensslPlugin::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString OpensslPlugin::pluginVersion() const
{
    return VERSION_STRING;
}

void OpensslPlugin::onGuiDelete()
{
    gui = 0;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(opensslplugin, OpensslPlugin)
#endif
QT_END_NAMESPACE
