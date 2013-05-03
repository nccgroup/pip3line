/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "microsofttimestamp.h"
#include "confgui/microsofttimestampwidget.h"
#include <QDebug>
#include <QDateTime>
#include <QDate>

const QString MicrosoftTimestamp::id = "Timestamp (Microsoft)";
const QString MicrosoftTimestamp::DEFAULT_DATE_FORMAT = "ddd d MMMM yyyy hh:mm:ss.zzz";
const QString MicrosoftTimestamp::PROP_DATEFORMAT = "DateFormat";

MicrosoftTimestamp::MicrosoftTimestamp()
{
    dateFormat = DEFAULT_DATE_FORMAT;
}

MicrosoftTimestamp::~MicrosoftTimestamp()
{
}

QString MicrosoftTimestamp::name() const
{
    return id;
}

QString MicrosoftTimestamp::description() const
{
    return tr("Translate Microsoft timestamp");
}

void MicrosoftTimestamp::transform(const QByteArray &input, QByteArray &output)
{
    if (input.isEmpty())
        return;

    output.clear();
    QDateTime timestamp;
    quint64 val1;
    quint64 val2;
    if (wayValue == INBOUND) {
        bool ok;

        val2 = input.toULongLong(&ok);
        quint64 rest = 0;
        if (ok) {

            val1 = val2 / (10000);
            rest = val2 % 10000;
            timestamp.setTimeSpec(Qt::UTC);
            timestamp.setDate(QDate(1601,1,1));
            timestamp = timestamp.addMSecs(val1 > LONG_MAX ? LONG_MAX : (qint64) val1);
            output = timestamp.toString(dateFormat).toUtf8();
            output.append("ms ").append(QByteArray::number(rest)).append(" ns UTC");
        } else {
            emit error(tr("Invalid number"),id);
        }
    } else {
        QByteArray temp = input;
        timestamp.setTimeSpec(Qt::UTC);
        timestamp.setDate(QDate(1601,1,1));
        val2 = qAbs(timestamp.toMSecsSinceEpoch());
        do  {
            timestamp = QDateTime::fromString(QString::fromUtf8(temp,temp.size()), dateFormat);
            temp.chop(1);
        } while (!timestamp.isValid() && temp.size() != 0);

        if (temp.size() == 0) {
            emit error(tr("Cannot parse the input"),id);
            return;
        }
        val2 = (val2 + timestamp.toMSecsSinceEpoch()) * 10000;

        output = QByteArray::number(val2);
    }
}

bool MicrosoftTimestamp::isTwoWays()
{
    return true;
}

QWidget *MicrosoftTimestamp::requestGui(QWidget * parent)
{
    QWidget * widget = new(std::nothrow) MicrosoftTimestampWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for MicrosoftTimestampWidget X{");
    }
    return widget;
}

QString MicrosoftTimestamp::help() const
{
    QString help;
    help.append("<p>Microsoft timestamp translator</p><p>For Microsoft Active Directory timestamps mostly, translate between uint64 and string representation.</p><p>Microsoft timestamps are usually UTC based, so setting to UTC.</p>");
    return help;
}

QHash<QString, QString> MicrosoftTimestamp::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_DATEFORMAT,dateFormat.toUtf8().toBase64());
    return properties;
}

bool MicrosoftTimestamp::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    setDateFormat(QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_DATEFORMAT).toUtf8())));

    return res;
}

QString MicrosoftTimestamp::inboundString() const
{
    return tr("Int to string");
}

QString MicrosoftTimestamp::outboundString() const
{
    return tr("String to int");
}

void MicrosoftTimestamp::setDateFormat(QString format)
{
    if (dateFormat != format) {
        dateFormat = format;
        emit confUpdated();
    }
}

QString MicrosoftTimestamp::getDateFormat() const
{
    return dateFormat;
}
