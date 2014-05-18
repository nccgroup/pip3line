/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef XMLCOMMON_H
#define XMLCOMMON_H
#include <QString>
#include <QByteArray>

#define LIB_TRANSFORM_VERSION 13

namespace Pip3lineConst {
    // Default types
    static const QString DEFAULT_TYPE_ENCODER = "Encoders";
    static const QString DEFAULT_TYPE_MISC = "Misc.";
    static const QString DEFAULT_TYPE_CRYPTO = "Crypto";
    static const QString DEFAULT_TYPE_HACKING = "Hacking";
    static const QString DEFAULT_TYPE_HASHES = "Hashes";
    static const QString DEFAULT_TYPE_TYPES_CASTING = "Types casting";
    static const QString DEFAULT_TYPE_PARSERS = "Parsers";
    static const QString DEFAULT_TYPE_NUMBER = "Numbers";
    static const QString DEFAULT_TYPE_USER = "User";

    // Default XML tags names
    static const QString XMLPIP3LINECONF = "pip3line";
    static const QString XMLDESCRIPTION = "Description";
    static const QString XMLHELP = "Help";
    static const QString XMLVERSIONMAJOR = "Version";
    static const QString XMLORDER = "Order";
    static const QString PROP_NAME = "Name";
    static const QString PROP_WAY = "Way";
    static const QString XMLTRANSFORM = "Transform";
    static const QString XMLVARIANT = "Variant";
    static const QString XMLPADDINGCHAR = "PaddingChar";
    static const QString XMLINCLUDEPADDING = "IncludePadding";
    static const QString XMLPADDINGTYPE = "PaddingType";
    static const QString XMLKEY = "Key";
    static const QString XMLFROMHEX = "FromHex";
    static const QString XMLPERCENTCHAR = "PercentChar";
    static const QString XMLINCLUDE = "Include";
    static const QString XMLEXCLUDE = "Exclude";
    static const QString XMLCHOOSENTARGET = "ChoosenTarget";
    static const QString XMLBLOCKSIZE = "BlockSize";
    static const QString XMLNOBLOCK = "NoBlock";
    static const QString XMLUSEENTITYNAMES = "UseEntityNames";
    static const QString XMLUSEHEXA = "UseHexadecimal";
    static const QString XMLENCODEALL = "EncodeAll";
    static const QString XMLISESCAPED = "IsEscaped";
    static const QString XMLCODECNAME = "CodecName";
    static const QString XMLCHAR62 = "Char62";
    static const QString XMLCHAR63 = "Char63";
    static const QString XMLFROM = "From";
    static const QString XMLLENGTH = "Length";
    static const QString XMLEVERYTHING = "Everything";
    static const QString XMLSEPARATOR = "Separator";
    static const QString XMLGROUP = "Group";
    static const QString XMLACTIONTYPE = "ActionType";
    static const QString XMLGREEDYQUANT = "UseGreedyQuantifiers";
    static const QString XMLREGEXP = "RegExpression";
    static const QString XMLCASEINSENSITIVE = "CaseInsensitive";
    static const QString XMLALLGROUPS = "AllGroups";
    static const QString XMLREPLACEWITH = "ReplaceWith";
    static const QString XMLBASE = "Base";
    static const QString XMLHASHNAME = "HashName";
    static const QString XMLUPPERCASE = "UpperCase";
    static const QString XMLPROCESSLINEBYLINE = "ProcessLineByLine";
    static const QString XMLTYPE = "Type";
    static const QString XMLLITTLEENDIAN = "LittleEndian";
    static const QString XMLQUERYSTRING = "XMLQuery";
    static const QString XMLLEVEL = "Level";
    static const QString XMLFORMAT = "Format";
    static const QString XMLDECODE = "Decode";
    static const QString XMLOFFSET = "Offset";
    static const QString XMLCLEAN = "Clean";
    static const QString XMLAUTORELOAD = "AutoReload";

    static const QString SETTINGS_SERVER_PORT = "DefaultServerPort";
    static const QString SETTINGS_SERVER_IP = "DefaultServerIP";
    static const QString SETTINGS_SERVER_PIPE_NAME = "DefaultServerPipe";
    static const QString SETTINGS_SERVER_DECODE = "DefaultServerDecode";
    static const QString SETTINGS_SERVER_ENCODE = "DefaultServerEncode";
    static const QString SETTINGS_SERVER_SEPARATOR = "DefaultServerSeparator";
    static const QString SETTINGS_PIPE_NAME="PipeName";
    static const QString UPDATE_URL = "https://raw.github.com/nccgroup/pip3line/master/gui/release.txt";
   // static const QString UPDATE_URL = "https://raw.githubusercontent.com/nccgroup/pip3line/master/gui/release.txt";
    static const QString SETTINGS_AUTO_UPDATE = "AutoUpdate";
    static const QString SETTINGS_GENERAL_GROUP = "General";
    static const QString SETTINGS_MINIMIZE_TO_TRAY = "MinimizeToTray";
    static const QString TCP_SERVER = "Tcp server";

    static const int DEFAULT_PORT = 45632;
    static const char DEFAULT_BLOCK_SEPARATOR = '\n';

#if defined(Q_OS_WIN32)
    static const QString DEFAULT_PIPE_MASS = "pip3lineMass";
    static const QString DEFAULT_PIPE = "pip3line";
    static const QString PIPE_SERVER = "Named Pipe Server";
    static const QString PIPE_SOCKET = "Named Pipe";
#else
    static const QString DEFAULT_PIPE_MASS = "/tmp/pip3lineMass";
    static const QString DEFAULT_PIPE = "/tmp/pip3line";
    static const QString PIPE_SERVER = "Unix Local Socket Server";
    static const QString PIPE_SOCKET = "Unix Local Socket";
#endif

    enum OutputFormat {TEXTFORMAT = 0, HEXAFORMAT = 1};

    static const QString USER_DIRECTORY = ".pip3line";
    static const int BLOCK_MAX_SIZE = 0x04000000;

    enum LOGLEVEL { LSTATUS = 0, LWARNING = 1, LERROR = 2};
    struct Message {
            QString message;
            QString source;
            LOGLEVEL level;
    };
    typedef QList<Message> Messages;
}

#endif // XMLCOMMON_H
