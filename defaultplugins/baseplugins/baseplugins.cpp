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
#include "bytestofloat.h"
#include "../../version.h"

#if QT_VERSION >= 0x050000
#include "sha224.h"
#include "sha256.h"
#include "sha384.h"
#include "sha512.h"
#endif

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
    TransformAbstract *ta = NULL;

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

#if QT_VERSION >= 0x050000

    if (Sha224::id == name) {
        ta = new(std::nothrow) Sha224();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Sha224 X{");
        }
    }

    if (Sha256::id == name) {
        ta = new(std::nothrow) Sha256();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Sha256 X{");
        }
    }

    if (Sha384::id == name) {
        ta = new(std::nothrow) Sha384();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Sha384 X{");
        }
    }

    if (Sha512::id == name) {
        ta = new(std::nothrow) Sha512();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Sha512 X{");
        }
    }

#endif

    if (BytesToFloat::id == name) {
        ta = new(std::nothrow) BytesToFloat();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for BytesToFloat X{");
        }
    }

    if (Ntlmssp::id == name) {
        ta = new(std::nothrow) Ntlmssp();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Ntlmssp X{");
        }
    }

    if (BaseX::id == name) {
        ta = new(std::nothrow) BaseX();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for BaseX X{");
        }
    }

    if (BytesInteger::id == name) {
        ta = new(std::nothrow) BytesInteger();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for BytesInteger X{");
        }
    }

    if (TimeStamp::id == name) {
        ta = new(std::nothrow) TimeStamp();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for TimeStamp X{");
        }
    }

    if (MicrosoftTimestamp::id == name) {
        ta = new(std::nothrow) MicrosoftTimestamp();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for MicrosoftTimestamp X{");
        }
    }

    if (name == Zlib::id) {
        ta = new(std::nothrow) Zlib();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Zlib X{");
        }
    }

    if (name == Hieroglyphy::id) {
        ta = new(std::nothrow) Hieroglyphy();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Hieroglyphy X{");
        }
    }

    if (name == NumberToChar::id) {
        ta = new(std::nothrow) NumberToChar();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for NumberToChar X{");
        }
    }

    if (name == Substitution::id) {
        ta = new(std::nothrow) Substitution();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Substitution X{");
        }
    }

    if (name == CiscoSecret7::id) {
        ta = new(std::nothrow) CiscoSecret7();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for CiscoSecret7 X{");
        }
    }

    if (name == FixProtocol::id) {
        ta = new(std::nothrow) FixProtocol();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for FixProtocol X{");
        }
    }

    if (name == IPTranslateIPv4::id) {
        ta = new(std::nothrow) IPTranslateIPv4();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for IPTranslateIPv4 X{");
        }
    }

    if (name == NetworkMaskIPv4::id) {
        ta = new(std::nothrow) NetworkMaskIPv4();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for NetworkMaskIPv4 X{");
        }
    }

    if (name == NetworkMaskIPv6::id) {
        ta = new(std::nothrow) NetworkMaskIPv6();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for NetworkMaskIPv6 X{");
        }
    }

    if (name == MySqlConcatv2::id) {
        ta = new(std::nothrow) MySqlConcatv2();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for MySqlConcatv2 X{");
        }
    }

    if (name == JavaScriptConcat::id) {
        ta = new(std::nothrow) JavaScriptConcat();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for JavaScriptConcat X{");
        }
    }

    if (name == MSSqlConcat::id) {
        ta = new(std::nothrow) MSSqlConcat();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for MSSqlConcat X{");
        }
    }

    if (name == PostgresConcat::id) {
        ta = new(std::nothrow) PostgresConcat();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for PostgresConcat X{");
        }
    }

    if (name == MysqlConcat::id) {
        ta = new(std::nothrow) MysqlConcat();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for MysqlConcat X{");
        }
    }

    if (name == OracleConcat::id) {
        ta = new(std::nothrow) OracleConcat();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for OracleConcat X{");
        }
    }

    if (name == UrlEncode::id) {
        ta = new(std::nothrow) UrlEncode();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for UrlEncode X{");
        }
    }

    if (name == Padding::id) {
        ta = new(std::nothrow) Padding();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Padding X{");
        }
    }

    if (name == Rotx::id) {
        ta = new(std::nothrow) Rotx();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Rotx X{");
        }
    }

    if (name == Cut::id) {
        ta = new(std::nothrow) Cut();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Cut X{");
        }
    }

    if (name == RandomCase::id) {
        ta = new(std::nothrow) RandomCase();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for RandomCase X{");
        }
    }

    if (name == RegularExp::id) {
        ta = new(std::nothrow) RegularExp();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for RegularExp X{");
        }
    }

    if (name == Split::id) {
        ta = new(std::nothrow) Split();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Split X{");
        }
    }

    if (name == Md5::id) {
        ta = new(std::nothrow) Md5();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Md5 X{");
        }
    }

    if (name == Sha1::id) {
        ta = new(std::nothrow) Sha1();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Sha1 X{");
        }
    }

    if (name == Md4::id) {
        ta = new(std::nothrow) Md4();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Md4 X{");
        }
    }

    if (name == Xor::id) {
        ta = new(std::nothrow) Xor();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Xor X{");
        }
    }

    if (name == CharEncoding::id) {
        ta = new(std::nothrow) CharEncoding();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for CharEncoding X{");
        }
    }

    if (name == Base64::id) {
        ta = new(std::nothrow) Base64();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Base64 X{");
        }
    }

    if (name == Base32::id) {
        ta = new(std::nothrow) Base32();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Base32 X{");
        }
    }

    if (name == Html::id) {
        ta = new(std::nothrow) Html();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Html X{");
        }
    }

    if (name == HexEncode::id) {
        ta = new(std::nothrow) HexEncode();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for HexEncode X{");
        }
    }

    if (name == Reverse::id) {
        ta = new(std::nothrow) Reverse();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for Reverse X{");
        }
    }

    if (name == Binary::id) {
        ta = new(std::nothrow) Binary();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for BasBinaryeX X{");
        }
    }

    if (name == XmlQuery::id) {
        ta = new(std::nothrow) XmlQuery();
        if (ta == NULL) {
            qFatal("Cannot allocate memory for XmlQuery X{");
        }
    }

    return ta;
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
#if QT_VERSION >= 0x050000
        enclist.append(Sha224::id);
        enclist.append(Sha256::id);
        enclist.append(Sha384::id);
        enclist.append(Sha512::id);
#endif
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
        enclist.append(BytesToFloat::id);
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

QWidget *BasePlugins::getConfGui(QWidget *)
{
    return NULL;
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
    return NULL;
}

QT_BEGIN_NAMESPACE
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(baseplugins, BasePlugins)
#endif
QT_END_NAMESPACE
