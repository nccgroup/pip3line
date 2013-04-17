/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef OPENSSLHASHES_H
#define OPENSSLHASHES_H

#include <QStringList>
#include <QMutex>
#include <transformabstract.h>
#include <openssl/evp.h>

class OpenSSLHashes : public TransformAbstract
{
        Q_OBJECT

    public:
        static QStringList hashList;
        explicit OpenSSLHashes(const QString &hashName = QString());
        ~OpenSSLHashes();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        static const QString id;

        QString getHashName();
        bool setHashName(QString name);
        QString help() const;
    private:
        QString hashName;
        EVP_MD_CTX *mdctx;
        const EVP_MD *md;
};
#endif // OPENSSLHASHES_H
