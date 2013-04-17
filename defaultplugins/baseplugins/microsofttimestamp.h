/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MICROSOFTTIMESTAMP_H
#define MICROSOFTTIMESTAMP_H

#include <transformabstract.h>

class MicrosoftTimestamp : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QString id;
        static const QString DEFAULT_DATE_FORMAT;
        static const QString PROP_DATEFORMAT;
        explicit MicrosoftTimestamp();
        ~MicrosoftTimestamp();
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

#endif // MICROSOFTTIMESTAMP_H
