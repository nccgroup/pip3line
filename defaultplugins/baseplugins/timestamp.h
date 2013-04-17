/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <transformabstract.h>

class TimeStamp : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const QString DEFAULT_DATE_FORMAT;
        static const QString PROP_DATEFORMAT;
        TimeStamp();
        ~TimeStamp();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QString help() const;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);

        QString inboundString() const;
        QString outboundString() const;

        void setDateFormat(QString format);
        QString getDateFormat() const;

    private:
        QString dateFormat;
};

#endif // TIMESTAMP_H
