/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "transformabstract.h"
#include <QTextStream>
#include <QDebug>

const QByteArray TransformAbstract::HEXCHAR("abcdefABCDEF1234567890");

TransformAbstract::TransformAbstract() {
    confGui = 0;
    wayValue = INBOUND;
    qDebug() << this;
}

TransformAbstract::~TransformAbstract() {

    if (confGui != 0)
        delete confGui;
    confGui = 0;
}

QByteArray TransformAbstract::transform(const QByteArray &input)
{
    QByteArray ret;
    transform(input,ret);
    return ret;
}

void TransformAbstract::logError(const QString message, const QString source)
{
    emit error(message, source);
}

void TransformAbstract::logWarning(const QString message, const QString source)
{
    emit warning(message, source);
}

bool TransformAbstract::isTwoWays()
{
    return false;
}

QWidget *TransformAbstract::getGui(QWidget * parent)
{
    if (confGui == 0) {
        confGui = requestGui(parent);
        if (confGui != 0) {
            connect(confGui, SIGNAL(destroyed()), this, SLOT(onGuiDelete()), Qt::UniqueConnection);
        }
    }
    return confGui;
}

QWidget *TransformAbstract::requestGui(QWidget * /* parent */)
{
    return 0;
}

void TransformAbstract::onGuiDelete()
{
    confGui = 0;
}

QString TransformAbstract::inboundString() const
{
    return tr("Encode");
}

QString TransformAbstract::outboundString() const
{
    return tr("Decode");
}

QString TransformAbstract::help() const
{
    return tr("No help available.<br> If you want to add help for your transformation, re-implement the help() function to return your own string. HTML is authorised.");
}

QString TransformAbstract::credits() const
{
    QString credits;
    credits.append(tr("<p>Released as open source by NCC Group Plc http://www.nccgroup.com/</p><p>Developped by Gabriel Caudrelier - gabriel.caudrelier@nccgroup.com</p><p>Released under AGPL see LICENSE file for more information</p> "));
    return credits;
}

QHash<QString, QString> TransformAbstract::getConfiguration()
{
    QHash<QString, QString> propertiesList;
    propertiesList.insert(PROP_WAY, QString::number((int)wayValue));
    propertiesList.insert(PROP_NAME, name());
    return propertiesList;
}

bool TransformAbstract::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool ok = true;
    int val = propertiesList.value(PROP_WAY).toInt(&ok);
    if (!ok || (val != INBOUND && val != OUTBOUND)) {
        emit error(tr("Invalid value for %1").arg(PROP_WAY),name());
        ok = false;
    }
    else {
        wayValue = (Way) val;
    }

    return ok;
}

void TransformAbstract::setWay(TransformAbstract::Way nway)
{
    wayValue = nway;
}

TransformAbstract::Way TransformAbstract::way()
{
    return wayValue;
}

QByteArray TransformAbstract::fromHex(QByteArray in)
{

    QString invalid;
    QString HEXCHAR("abcdefABCDEF0123456789");
    for (int i = 0; i < in.size(); i++){
        if (!HEXCHAR.contains(in.at(i))) {
            if (!invalid.contains(in.at(i)))
                invalid.append(in.at(i));
        }
    }

    if (!invalid.isEmpty()) {
        emit error(tr("Invalid character(s) found in the hexadecimal stream: '%1', they will be skipped").arg(invalid),name());
    }

    in.replace(invalid,"");

    if (in.size()%2 != 0) {
        in.chop(1);
        emit error(tr("Invalid size for a hexadecimal stream, skipping the last byte."),name());
    }

    return QByteArray::fromHex(in);
}

QByteArray TransformAbstract::fromEscapedHex(const QByteArray &val)
{
    QByteArray temp;
    int i = 0;
    while (i < val.size()) {
        if (val.at(i) == '\\' && i < val.size() - 3 &&  val.at(i+1) == 'x') {
            if (HEXCHAR.contains(val.at(i+2)) && HEXCHAR.contains(val.at(i+3))) {
                temp.append(val.at(i+2)).append(val.at(i+3));
                i += 4;
                continue;
            }
        }
        i++;
    }
    return QByteArray::fromHex(temp);
}

QByteArray TransformAbstract::fromCStyleArray(const QByteArray &val)
{
    QByteArray temp;
    int i = 0;
    while (i < val.size()) {
        if (val.at(i) == '0' && i < val.size() - 3 &&  val.at(i+1) == 'x') {
            if (HEXCHAR.contains(val.at(i+2)) && HEXCHAR.contains(val.at(i+3))) {
                temp.append(val.at(i+2)).append(val.at(i+3));
                i += 4;
                continue;
            }
        }
        i++;
    }
    return QByteArray::fromHex(temp);
}

QByteArray TransformAbstract::toEscapedHex(const QByteArray &val)
{
    QByteArray final;

    for (int i = 0; i < val.size(); i++) {
        final.append("\\x").append(val.mid(i,1).toHex());
    }

    return final;
}

QByteArray TransformAbstract::toCStyleArray(const QByteArray &val)
{
    QByteArray final;
    final.append("{");
    for (int i = 0; i < val.size(); i++) {
        final.append(" 0x").append(val.mid(i,1).toHex()).append(",");
    }
    final.chop(1);
    final.append(" }");
    return final;
}

QByteArray TransformAbstract::toCSV(const QByteArray &val)
{
    QByteArray final;

    for (int i = 0; i < val.size(); i++) {
        final.append(val.mid(i,1).toHex()).append(",");
    }
    return final;
}

QByteArray TransformAbstract::toPrintableString(const QByteArray &val)
{
    QByteArray ret;
    for (int i = 0; i < val.size(); i++) {
        char c = val.at(i);
        if (c > ' ' && c <= '~') {
            ret.append(c);
        } else {
            switch (c) {
                case '\n':
                    ret.append("\\n");
                    break;
                case '\r':
                    ret.append("\\r");
                    break;
                default:
                ret.append("\\").append(QByteArray(1,c).toHex());
            }
        }
    }

    return ret;
}
