/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QTextStream>
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include "baseplugins.h"
#include "urlencode.h"
#include "md5.h"
#include "md4.h"
#include "sha1.h"
#include "xor.h"
#include "charencoding.h"
#include "base64.h"
#include "base32.h"
#include "hexencode.h"
#include "reverse.h"
#include "binary.h"
#include "padding.h"
#include "html.h"
#include "rotx.h"
#include "cut.h"
#include "split.h"
#include "regularexp.h"
#include "randomcase.h"
#include "oracleconcat.h"
#include "mysqlconcat.h"
#include "postgresconcat.h"
#include "mssqlconcat.h"
#include "javascriptconcat.h"
#include "mysqlconcatv2.h"
#include "xmlquery.h"
#include "iptranslateipv4.h"
#include "networkmaskipv4.h"
#include "networkmaskipv6.h"
#include "fixprotocol.h"
#include "ciscosecret7.h"
#include "substitution.h"
#include "numbertochar.h"
#include "hieroglyphy.h"
#include "zlib.h"
#include "basex.h"
#include "microsofttimestamp.h"
#include "timestamp.h"
#include "bytesinteger.h"
#include "ntlmssp.h"
#include "../../version.h"

const QString BasePlugins::Base64Url = "Base64 && Url Encode";
const QString BasePlugins::BinaryNum = "Binary (num)";
const QString BasePlugins::OctalNum = "Octal (num)";
const QString BasePlugins::HexaNum = "Hexadecimal (num)";

BasePlugins::BasePlugins()
{
}


BasePlugins::~BasePlugins()
{
    qDebug() << "Destroying " << this;
}

QString BasePlugins::pluginName() const
{
    return tr("Base Transformations");
}

TransformAbstract *BasePlugins::getTransform(QString name)
{
    if (name == Base64Url) {
        return getTransformFromFile(":/harcoded/composedxml/base64UrlEncode.xml");
    }

    if (name == BinaryNum) {
        return getTransformFromFile(":/harcoded/composedxml/binarynumber.xml");
    }

    if (name == OctalNum) {
        return getTransformFromFile(":/harcoded/composedxml/octalnumber.xml");
    }

    if (name == HexaNum) {
        return getTransformFromFile(":/harcoded/composedxml/hexanumber.xml");
    }

    if (Ntlmssp::id == name)
        return new Ntlmssp();

    if (BaseX::id == name)
        return new BaseX();

    if (BytesInteger::id == name)
        return new BytesInteger();

    if (TimeStamp::id == name)
        return new TimeStamp();

    if (MicrosoftTimestamp::id == name)
        return new MicrosoftTimestamp();

    if (name == Zlib::id)
        return new Zlib();

    if (name == Hieroglyphy::id)
        return new Hieroglyphy();

    if (name == NumberToChar::id)
        return new NumberToChar();

    if (name == Substitution::id)
        return new Substitution();

    if (name == CiscoSecret7::id)
        return new CiscoSecret7();

    if (name == FixProtocol::id)
        return new FixProtocol();

    if (name == IPTranslateIPv4::id)
        return new IPTranslateIPv4();

    if (name == NetworkMaskIPv4::id)
        return new NetworkMaskIPv4();

    if (name == NetworkMaskIPv6::id)
        return new NetworkMaskIPv6();

    if (name == MySqlConcatv2::id)
        return new MySqlConcatv2();

    if (name == JavaScriptConcat::id)
        return new JavaScriptConcat();

    if (name == MSSqlConcat::id)
        return new MSSqlConcat();

    if (name == PostgresConcat::id)
        return new PostgresConcat();

    if (name == MysqlConcat::id)
        return new MysqlConcat();

    if (name == OracleConcat::id)
        return new OracleConcat();

    if (name == UrlEncode::id)
        return new UrlEncode();

    if (name == Padding::id)
        return new Padding();

    if (name == Rotx::id)
        return new Rotx();

    if (name == Cut::id)
        return new Cut();

    if (name == RandomCase::id)
        return new RandomCase();

    if (name == RegularExp::id)
        return new RegularExp();

    if (name == Split::id)
        return new Split();
    if (name == Md5::id)
        return new Md5();

    if (name == Sha1::id)
        return new Sha1();

    if (name == Md4::id)
        return new Md4();
    if (name == Xor::id)
        return new Xor();

    if (name == CharEncoding::id)
        return new CharEncoding();

    if (name == Base64::id)
        return new Base64();

    if (name == Base32::id)
        return new Base32();

    if (name == Html::id)
        return new Html();

    if (name == HexEncode::id)
        return new HexEncode();

    if (name == Reverse::id)
        return new Reverse();

    if (name == Binary::id)
        return new Binary();

    if (name == XmlQuery::id)
        return new XmlQuery();

    return 0;
}

const QStringList BasePlugins::getTransformList(QString typeName)
{
    QStringList enclist;

    if (typeName == DEFAULT_TYPE_ENCODER) {
        enclist.append(UrlEncode::id);
        enclist.append(CharEncoding::id);
        enclist.append(Base64::id);
        enclist.append(Base64Url);
        enclist.append(Base32::id);
        enclist.append(HexEncode::id);
        enclist.append(Binary::id);
        enclist.append(Html::id);
        enclist.append(NumberToChar::id);
    } else if (typeName == DEFAULT_TYPE_HASHES) {
        enclist.append(Md5::id);
        enclist.append(Md4::id);
        enclist.append(Sha1::id);
    } else if (typeName == DEFAULT_TYPE_CRYPTO) {
        enclist.append(Xor::id);
        enclist.append(Rotx::id);
        enclist.append(CiscoSecret7::id);
        enclist.append(Substitution::id);
    } else if (typeName == DEFAULT_TYPE_HACKING) {
        enclist.append(Hieroglyphy::id);
        enclist.append(RandomCase::id);
        enclist.append(OracleConcat::id);
        enclist.append(MysqlConcat::id);
        enclist.append(PostgresConcat::id);
        enclist.append(MSSqlConcat::id);
        enclist.append(JavaScriptConcat::id);
        enclist.append(MySqlConcatv2::id);
    } else if (typeName == DEFAULT_TYPE_PARSERS) {
        enclist.append(FixProtocol::id);
        enclist.append(Ntlmssp::id);
    } else if (typeName == DEFAULT_TYPE_MISC) {
        enclist.append(Reverse::id);
        enclist.append(Padding::id);
        enclist.append(Cut::id);
        enclist.append(Split::id);
        enclist.append(RegularExp::id);
        enclist.append(XmlQuery::id);
        enclist.append(NetworkMaskIPv4::id);
        enclist.append(NetworkMaskIPv6::id);
        enclist.append(Zlib::id);
    } else if (typeName == DEFAULT_TYPE_TYPES_CASTING) {
        enclist.append(IPTranslateIPv4::id);
        enclist.append(MicrosoftTimestamp::id);
        enclist.append(TimeStamp::id);
        enclist.append(BytesInteger::id);
    } else if (typeName == DEFAULT_TYPE_NUMBER) {
        enclist.append(BaseX::id);
        enclist.append(BinaryNum);
        enclist.append(OctalNum);
        enclist.append(HexaNum);
    }

    return enclist;
}

const QStringList BasePlugins::getTypesList()
{
    return QStringList() << DEFAULT_TYPE_ENCODER << DEFAULT_TYPE_CRYPTO << DEFAULT_TYPE_MISC << DEFAULT_TYPE_PARSERS
                         << DEFAULT_TYPE_HASHES << DEFAULT_TYPE_HACKING << DEFAULT_TYPE_TYPES_CASTING << DEFAULT_TYPE_NUMBER;
}

QWidget *BasePlugins::getConfGui(QWidget */*parent*/)
{
    return 0;
}

QString BasePlugins::compiledWithQTversion() const
{
    return QT_VERSION_STR;
}

int BasePlugins::getLibTransformVersion() const
{
    return LIB_TRANSFORM_VERSION;
}

QString BasePlugins::pluginVersion() const
{
    return VERSION_STRING;
}

void BasePlugins::setCallBack(Pip3lineCallback * ncallback)
{
    callback = ncallback;
}

TransformAbstract *BasePlugins::getTransformFromFile(QString resFile)
{
    QFile source(resFile);
    if (source.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&source);
        return callback->getTransformFromXML(&reader);
    } else {
        callback->logError(source.errorString());
    }
    return 0;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(baseplugins, BasePlugins)
#endif
QT_END_NAMESPACE
