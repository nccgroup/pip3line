/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QColor>
#include <QFont>
#include <QStringList>

#ifndef GUICONFIGURATION_H
#define GUICONFIGURATION_H

namespace GuiConst
{
    static const QString UNDEFINED_TEXT = "Undefined";
    static const QString SETTINGS_SERVER_PORT = "DefaultServerPort";
    static const QString SETTINGS_SERVER_IP = "DefaultServerIP";
    static const QString SETTINGS_SERVER_PIPE_NAME = "DefaultServerPipe";
    static const QString SETTINGS_SERVER_DECODE = "DefaultServerDecode";
    static const QString SETTINGS_SERVER_ENCODE = "DefaultServerEncode";
    static const QString SETTINGS_SERVER_SEPARATOR = "DefaultServerSeparator";
    static const QString SETTINGS_PIPE_NAME="PipeName";
    static const QString SETTINGS_QUICKVIEWS = "QuickView";
    static const QString SETTINGS_FILTER_BLACKLIST = "FilterBlacklist";
    static const QString SETTINGS_MARKINGS_COLORS = "MarkingsColors";
    static const QString SETTINGS_EXPORT_IMPORT_FUNC = "ExportImportFunctions";
    static const QString SETTINGS_OFFSET_BASE = "OffsetBase";
    static const QString SETTINGS_GLOBAL_PROXY_IP = "GlobalProxyIP";
    static const QString SETTINGS_GLOBAL_PROXY_PORT = "GlobalProxyPort";
    static const QString SETTINGS_AUTO_COPY_TRANSFORM = "AutoCopyTransform";
    static const QString SETTINGS_AUTO_UPDATE = "AutoUpdate";
    static const QString SETTINGS_GENERAL_GROUP = "General";
    static const QString SETTINGS_MINIMIZE_TO_TRAY = "MinimizeToTray";
    static const QString SETTINGS_MASS_PROCESSING_GROUP = "MassProcessing";
    static const QString SETTINGS_DEFAULT_TAB = "DefaultTab";
    static const QString SETTINGS_IGNORE_SSL_ERRORS = "IgnoreSSLErrors";
    static const QString SETTINGS_ENABLE_NETWORK_PROXY = "EnableNetworkProxy";
    static const QString SETTINGS_DEFAULT_SAVELOAD_FLAGS = "DefaultSaveLoadFlags";
    static const QString SETTINGS_AUTO_SAVE_ENABLE = "AutoSaveEnabled";
    static const QString SETTINGS_AUTO_SAVE_SINGLE_FILENAME = "AutoSaveSingleFileName";
    static const QString SETTINGS_AUTO_SAVE_FILENAME = "autostate.sav";
    static const QString SETTINGS_AUTO_SAVE_ON_EXIT = "AutoSaveOnExit";
    static const QString SETTINGS_AUTO_SAVE_TIMER_ENABLE = "AutoSaveTimerEnable";
    static const QString SETTINGS_AUTO_SAVE_TIMER_INTERVAL = "AutoSaveTimerInterval";
    static const QString SETTINGS_AUTO_RESTORE_ON_STARTUP = "AutoRestoreOnStartup";

    static const QString DEFAULT_STATE_FILE = "savedstate.conf";
    static const QString STATE_PIP3LINE_DOC = "Pip3lineState";
    static const QString STATE_MAIN_WINDOW = "MainWindow";
    static const QString STATE_DIALOGS_RUNNING = "DialogsRunning";
    static const QString STATE_YES = "Yes";
    static const QString STATE_NO = "No";
    static const QString STATE_WIDGET_GEOM = "Geometry";
    static const QString STATE_WIDGET_STATE = "State";
    static const QString STATE_GLOBAL_CONF = "GlobalConf";
    static const QString STATE_MAIN_WINDOW_STATE = "MainWindowState";
    static const QString STATE_SETTINGS_DIALOG = "SettingsDialog";
    static const QString STATE_ANALYSE_DIALOG = "AnalyseDialog";
    static const QString STATE_REGEXPHELP_DIALOG = "RegExpHelpDialog";
    static const QString STATE_QUICKVIEW_DIALOG = "QuickView";
    static const QString STATE_QUICKVIEW_CONF = "QuickViewConf";
    static const QString STATE_QUICKVIEW_ITEM_COUNT = "QuickViewItemCount";
    static const QString STATE_QUICKVIEW_ITEM = "QuickViewItem";
    static const QString STATE_CONF = "Conf";
    static const QString STATE_COMPARISON_DIALOG = "ComparisonDialog";
    static const QString STATE_UI_CONF = "UIConf";
    static const QString STATE_MARKING_COLOR = "MarkingColor";
    static const QString STATE_DEBUG_DIALOG = "DebugDialog";
    static const QString STATE_DIALOG_GEOM = "DialogGeom";
    static const QString STATE_DIALOG_ISVISIBLE = "IsVisible";
    static const QString STATE_INPUT_CONTENT = "InputContent";
    static const QString STATE_OUTPUT_CONTENT = "OutputContent";
    static const QString STATE_MAINTABS = "MainTabs";
    static const QString STATE_MAINTABS_LOGGER = "MainTabsLogger";
    static const QString STATE_CURRENT_INDEX = "CurrentIndex";
    static const QString STATE_IS_FOLDED = "IsFolded";
    static const QString STATE_SCROLL_INDEX = "ScrollIndex";
    static const QString STATE_RANDOM_SOURCE_CURRENT_OFFSET = "CurrentStartingOffset";
    static const QString STATE_CHUNK_SIZE = "ChunkSize";
    static const QString STATE_LARGE_FILE_NAME = "LargeFileName";
    static const QString STATE_SEARCH_WIDGET = "SearchData";
    static const QString STATE_GOTOOFFSET_WIDGET = "GotoOffsetData";
    static const QString STATE_MESSAGE_PANEL = "Messages";
    static const QString STATE_MESSAGE_PANEL_VISIBLE = "MessPanelVisible";
    static const QString STATE_SIZE = "Size";
    static const QString STATE_TABS_ARRAY = "Tabs";
    static const QString STATE_WINDOWED_TABS_ARRAY = "WindowTabs";
    static const QString STATE_WINDOWED = "Windowed";
    static const QString STATE_TYPE = "Type";
    static const QString STATE_TRANSFORMGUI_ARRAY = "TransformGuiList";
    static const QString STATE_TAB_NAME = "TabName";
    static const QString STATE_DATA = "Data";
    static const QString STATE_BYTESOURCE = "ByteSource";
    static const QString STATE_TAB = "Tab";
    static const QString STATE_TABVIEWLIST = "TabViewList";
    static const QString STATE_TABVIEW = "TabView";
    static const QString STATE_PRETAB_TYPE = "PreTab";
    static const QString STATE_NAME = "Name";
    static const QString STATE_READONLY = "Readonly";
    static const QString STATE_STATIC_MARKINGS = "StaticMarkings";
    static const QString STATE_USERMARKINGS = "UserMarkings";
    static const QString STATE_MARKING = "Marking";
    static const QString STATE_HEX_HISTORY = "HexHistory";
    static const QString STATE_HEX_HISTORY_ITEM = "HexHistoryItem";
    static const QString STATE_HEX_HISTORY_POINTER = "HexHistoryPointer";
    static const QString STATE_HEX_HISTORY_OFFSET = "HexHistoryOffset";
    static const QString STATE_HEX_HISTORY_ACTION = "HexHistoryAction";
    static const QString STATE_HEX_HISTORY_BEFORE = "HexHistoryBefore";
    static const QString STATE_HEX_HISTORY_AFTER = "HexHistoryAfter";
    static const QString STATE_BYTE_DESC = "ByteRangeDesc";
    static const QString STATE_BYTE_LOWER_VAL = "ByteRangeLowerVal";
    static const QString STATE_BYTE_UPPER_VAL = "ByteRangeUpperVal";
    static const QString STATE_BYTE_FG_COLOR = "ByteRangeForegroundColor";
    static const QString STATE_BYTE_BG_COLOR = "ByteRangeBackgroundColor";
    static const QString STATE_TRANSFORM_CONF = "TransformConf";
    static const QString STATE_CAPABILITIES = "Caps";
    static const quint64 STATE_SAVE_REQUEST = 0x1;
    static const quint64 STATE_LOADSAVE_DIALOG_POS = 0x2;
    static const quint64 STATE_LOADSAVE_DATA = 0x4;
    static const quint64 STATE_LOADSAVE_SEARCH_RESULTS = 0x8;
    static const quint64 STATE_LOADSAVE_QUICKVIEW_CONF = 0x10;
    static const quint64 STATE_LOADSAVE_HISTORY = 0x20;
    static const quint64 STATE_LOADSAVE_MARKINGS = 0x40;
    static const quint64 STATE_LOADSAVE_COMPARISON = 0x80;
    static const quint64 STATE_LOADSAVE_GLOBAL_CONF = 0x100;
    static const quint64 STATE_LOADSAVE_LOAD_ALL = 0xFFFFFFFFFFFFFFFE;
    static const quint64 STATE_LOADSAVE_SAVE_ALL = 0xFFFFFFFFFFFFFFFF;
    static const QString STATE_LOGGER_INDEX = "LoggerTabIndex";

    static const bool DEFAULT_AUTO_SAVE_ENABLED = true;
    static const bool DEFAULT_AUTO_RESTORE_ENABLED = true;
    static const bool DEFAULT_AUTO_SAVE_ON_EXIT = true;
    static const bool DEFAULT_AUTO_SAVE_TIMER_ENABLE = true;
    static const int DEFAULT_AUTO_SAVE_TIMER_INTERVAL = 5;
    static const int MAX_AUTO_SAVE_TIMER_INTERVAL = 120;
    static const int MIN_AUTO_SAVE_TIMER_INTERVAL = 1;

    static const QString STATE_ACTION_RESTORE_STR = "restoring";
    static const QString STATE_ACTION_SAVE_STR = "saving";

    static const QString TCP_SERVER = QObject::tr("Tcp server");

    static const QString UPDATE_URL = "https://raw.githubusercontent.com/nccgroup/pip3line/master/gui/release.txt";
    static const QString RELEASES_URL = "https://github.com/nccgroup/pip3line/releases";
    static const QString NO_TRANSFORM = "No transform";

    static const int DEFAULT_PORT = 45632;
    static const char DEFAULT_BLOCK_SEPARATOR = '\n';
    static const bool DEFAULT_SERVER_ENCODE = false;
    static const bool DEFAULT_SERVER_DECODE = false;

    static const QString LOGS_STR = "Logs";

#if defined(Q_OS_WIN32)
    static const QString DEFAULT_PIPE_MASS = "pip3lineMass";
    static const QString DEFAULT_PIPE = "pip3line";
    static const QString PIPE_SERVER = QObject::tr("Named Pipe Server");
    static const QString PIPE_SOCKET = QObject::tr("Named Pipe");
#else
    static const QString DEFAULT_PIPE_MASS = "/tmp/pip3lineMass";
    static const QString DEFAULT_PIPE = "/tmp/pip3line";
    static const QString PIPE_SERVER = QObject::tr("Unix Local Socket Server");
    static const QString PIPE_SOCKET = QObject::tr("Unix Local Socket");
#endif

    static const bool DEFAULT_AUTO_COPY_TEXT = false;
    static const bool DEFAULT_IGNORE_SSL = true;
    static const bool DEFAULT_PROXY_ENABLE = false;
    static const QString DEFAULT_GLOBAL_PROXY_IP = "127.0.0.1";
    static const quint16 DEFAULT_GLOBAL_PROXY_PORT = 8080;
    static const int DEFAULT_OFFSET_BASE = 16;

    static const QString UTF8_STRING_ACTION = QObject::tr("UTF-8");
    static const QString NEW_BYTE_ACTION = QObject::tr("New Byte(s)");
    static const QString SEND_TO_NEW_TAB_ACTION = QObject::tr("New tab");

    static const QString LITTLE_ENDIAN_STRING = QObject::tr("Little endian");
    static const QString BIG_ENDIAN_STRING = QObject::tr("Big endian");
    static const QString OCTAL_STRING = QObject::tr("Octal");
    static const QString DECIMAL_STRING = QObject::tr("Decimal");
    static const QString HEXADECIMAL_STRING = QObject::tr("Hexadecimal");
    static const QString ABSOLUTE_LITTLE_ENDIAN_STRING = QObject::tr("Absolute LE");
    static const QString ABSOLUTE_BIG_ENDIAN_STRING = QObject::tr("Absolute BE");
    static const QString RELATIVE_LITTLE_ENDIAN_STRING = QObject::tr("Relative LE");
    static const QString RELATIVE_BIG_ENDIAN_STRING = QObject::tr("Relative BE");

    static const uint DEFAULT_MAX_TAB_COUNT = 50;

    static const int MAX_DELETED_TABS_KEPT = 30;

    enum TAB_TYPES {
        TRANSFORM_TAB_TYPE = 0,  // default
        GENERIC_TAB_TYPE = 1,
        RANDOM_ACCESS_TAB_TYPE = 2
    };

    enum AVAILABLE_PRETABS {
        INVALID_PRETAB = -1,
        TRANSFORM_PRETAB = 0,
        LARGE_FILE_PRETAB = 1,
        CURRENTMEM_PRETAB = 2,
        HEXAEDITOR_PRETAB = 3,
        INTERCEPT_PRETAB = 4
    };

    static const QString TRANSFORM_TAB_STRING = QObject::tr("Transform Editor");
    static const QString LARGE_FILE_TAB_STRING = QObject::tr("Large File Editor");
    static const QString CURRENTMEM_TAB_STRING = QObject::tr("Current memory");
    static const QString BASEHEX_TAB_STRING = QObject::tr("Hexeditor");
    static const QString INTERCEP_TAB_STRING = QObject::tr("Interceptor");
    static const QStringList AVAILABLE_TAB_STRINGS = QStringList() << TRANSFORM_TAB_STRING
                                                                 << LARGE_FILE_TAB_STRING
                                                                 << CURRENTMEM_TAB_STRING
                                                                 << BASEHEX_TAB_STRING
                                                                 << INTERCEP_TAB_STRING;

    static const QString COPYOFFSET = QObject::tr("Copy Offset as");
    static const QString SAVELISTTOFILE = QObject::tr("Save List to file as");
    static const QString COPYLIST = QObject::tr("Copy list as");
}

namespace GuiStyles {
    static const QString LineEditError = "QLineEdit { background-color: #FFB1B2; }";
    static const QString ComboBoxError = "QComboBox { color : red; }";
    static const QString PushButtonReadonly = "QPushButton { color : #FF0000; }";
    static const QString LineEditWarning = "";
    static const QString LineEditMessage = "";
    static const QColor DEFAULT_MARKING_COLOR = QColor(255,182,117);
    static const QColor DEFAULT_MARKING_COLOR_DATA = QColor(11449599);
    static const QColor DEFAULT_MARKING_COLOR_SIZE = QColor(16755616);
    static const QColor DEFAULT_MARKING_COLOR_TYPE = QColor(9043881);
    static const QFont DEFAULT_REGULAR_FONT = QFont("Courier New",10);
}

#endif // GUICONST_H
