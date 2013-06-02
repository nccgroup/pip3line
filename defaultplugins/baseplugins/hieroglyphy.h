/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HIEROGLYPHY_H
#define HIEROGLYPHY_H

#include <transformabstract.h>
#include <QStringList>

class Hieroglyphy : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit Hieroglyphy();
        ~Hieroglyphy();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        QString inboundString() const;
        QString outboundString() const;
        QString hChar(char c);
        QString hUnicode(QChar u);
        QString hNumber(int num);
        QString hString(QString str);
        QString hScript(QString scr);
        QString toHex(char c);
        QString help() const;
        QString credits() const;

        void setUseBtoa(bool val);
        bool isBtoaInUse() const;
    private:
        static const QString XMLBTOAINUSE;
        void init();
        void btoaInit();
        QStringList numbers;
        QHash<char, QString> characters;
        QString Sobject_Object;
        QString SNaN;
        QString Strue;
        QString Sfalse;
        QString Sundefined;
        QString SInfinity;
        QString S1e100;
        QString functionConstructor;
        QString locationString;
        QString unescapeString;
        QString escapeString;
        QString fromCharCodeS;
        QString btoaFunction;
        bool btoaInUse;
};

#endif // HIEROGLYPHY_H
