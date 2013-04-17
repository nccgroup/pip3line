/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "zlib.h"
#include "confgui/zlibwidget.h"

const QString Zlib::id = "Zlib";
const QString Zlib::XMLREMOVEHEADER = "RemoveSizeHeader";

Zlib::Zlib()
{
    compressionLevel = 1;
    removeHeader = false;
}

QString Zlib::name() const
{
    return id;
}

QString Zlib::description() const
{
    return tr("Decompression with zlib");
}

void Zlib::transform(const QByteArray &input, QByteArray &output)
{
    if (wayValue == INBOUND) {
        output = qCompress(input,compressionLevel);
        if (removeHeader)
            output = output.mid(4);
    } else {
        output = qUncompress(input);
        if (output.size() == 0) {
            QByteArray temp(4,0x00);
            output = qUncompress(temp.append(input));
        }
    }
}

bool Zlib::isTwoWays()
{
    return true;
}

QHash<QString, QString> Zlib::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLLEVEL,QString::number((int)compressionLevel));
    properties.insert(XMLREMOVEHEADER,QString::number((int)removeHeader));

    return properties;
}

bool Zlib::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok = true;
    int val = propertiesList.value(XMLLEVEL).toInt(&ok);
    if (!ok || val < 0 || val > 9) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLLEVEL),id);
    } else {
        setCompression(val);
    }

    val = propertiesList.value(XMLREMOVEHEADER).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLREMOVEHEADER),id);
    } else {
        setRemoveHeader(val == 1);
    }

    return res;
}

QWidget *Zlib::requestGui(QWidget * parent)
{
    return new ZlibWidget(this, parent);
}

QString Zlib::help() const
{
    QString help;
    help.append("<p>Zlib compression algorithm</p><p>When compressing there is an option to automatically remove the size header (first 4 bytes), which is not used in many cases.</p>");
    return help;
}

QString Zlib::inboundString() const
{
    return tr("Compress");
}

QString Zlib::outboundString() const
{
    return tr("Decompress");
}

bool Zlib::setCompression(int level)
{
    if (level < 0 && level > 9) {
        emit error(tr("Incorrect compression level"),id);
        return false;
    }

    compressionLevel = level;

    emit confUpdated();
    return true;
}

int Zlib::getCompression()
{
    return compressionLevel;
}

void Zlib::setRemoveHeader(bool flag)
{
    removeHeader = flag;
    emit confUpdated();
}

bool Zlib::doRemoveHeader()
{
    return removeHeader;
}
