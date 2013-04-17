/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "timestamp.h"
#include "confgui/timestampwidget.h"
#include <QDateTime>

const QString TimeStamp::id = "Timestamp (Epoch)";
const QString TimeStamp::DEFAULT_DATE_FORMAT = "dd/MM/yyyy hh:mm:ss";
const QString TimeStamp::PROP_DATEFORMAT = "DateFormat";

TimeStamp::TimeStamp()
{
    dateFormat = DEFAULT_DATE_FORMAT;
}

TimeStamp::~TimeStamp()
{

}

QString TimeStamp::name() const
{
    return id;
}

QString TimeStamp::description() const
{
    return tr("Epoch timestamp translator");
}

void TimeStamp::transform(const QByteArray &input, QByteArray &output)
{
    QDateTime timestamp;
    if (wayValue == INBOUND) {
        bool ok;
        emit warning(tr("Setting the timezone to UTC."),id);
        timestamp.setTimeSpec(Qt::UTC);
        uint val1 = input.toUInt(&ok);
        if (ok) {
            timestamp.setTime_t(val1);
            output = timestamp.toString(dateFormat).toUtf8();
        } else {
            qint64 val2 = input.toLongLong(&ok);
            if (ok) {
                timestamp.setMSecsSinceEpoch(val2);
                output = timestamp.toString(dateFormat).toUtf8();
            } else {
                emit error(tr("could not parse the number"),id);
                return;
            }
        }
        output.append(" UTC");
    } else {
        QByteArray temp = input;
        do  {
            timestamp = QDateTime::fromString(QString::fromUtf8(temp,temp.size()), dateFormat);
            temp.chop(1);
        } while (!timestamp.isValid() && temp.size() != 0);

        if (temp.size() == 0) {
            emit error(tr("Cannot parse the input"),id);
            return;
        }

        output = QByteArray::number(timestamp.toTime_t());
    }
}

bool TimeStamp::isTwoWays()
{
    return true;
}

QWidget *TimeStamp::requestGui(QWidget * parent)
{
    return new TimestampWidget(this, parent);
}

QString TimeStamp::help() const
{
    QString help;
    help.append("<p>Epoch timestamp translator</p><p>For Unix timestamp mostly, translate between integer and string representation.</p><p> Warning: There is no assumption on the timezone at all, it could be anything. Setting it to UTC by default</p>");
    return help;
}

QHash<QString, QString> TimeStamp::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(PROP_DATEFORMAT,dateFormat.toUtf8().toBase64());
    return properties;
}

bool TimeStamp::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    setDateFormat(QString::fromUtf8(QByteArray::fromBase64(propertiesList.value(PROP_DATEFORMAT).toUtf8())));

    return res;
}

QString TimeStamp::inboundString() const
{
    return tr("Int to string");
}

QString TimeStamp::outboundString() const
{
    return tr("String to int");
}

void TimeStamp::setDateFormat(QString format)
{
    if (dateFormat != format) {
        dateFormat = format;
        emit confUpdated();
    }
}

QString TimeStamp::getDateFormat() const
{
    return dateFormat;
}



