/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTESOURCEABSTRACT_H
#define BYTESOURCEABSTRACT_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QColor>
#include <QMap>
#include <commonstrings.h>
#include "searchabstract.h"

class QWidget;

class OffsetsRange
{
    public:
        static const QString HEXFORMAT;
        explicit OffsetsRange(quint64 lowerVal, quint64 upperVal, QString description = QString());
        OffsetsRange(const OffsetsRange& other);
        OffsetsRange &operator=(const OffsetsRange &other);
        virtual ~OffsetsRange();
        bool isInRange(int value);
        bool isInRange(quint64 value);
        QString getDescription() const ;
        void setDescription(const QString &descr);
        QColor getForeground() const;
        void setForeground(const QColor &color);
        QColor getBackground() const;
        void setBackground(const QColor &color);
        quint64 getLowerVal() const;
        void setLowerVal(quint64 val);
        quint64 getUpperVal() const;
        void setUpperVal(quint64 val);
        virtual bool operator<(const OffsetsRange& other) const;
        bool sameMarkings(const OffsetsRange& other) const;
        static QString offsetToString(quint64 val);
        static bool lessThanFunc(OffsetsRange * or1, OffsetsRange *or2);
        quint64 getSize() const;
        void setSize(const quint64 &value);
    protected:
        QString description;
        quint64 lowerVal;
        quint64 upperVal;
        quint64 size;
        QColor foregroundColor;
        QColor backgroundColor;

    friend class ByteSourceAbstract;
    friend class ComparableRange;
};

class ComparableRange
{
    public:
        explicit ComparableRange(OffsetsRange * pointer);
        OffsetsRange *getRange() const;
        void setRange(OffsetsRange *value);
        bool operator<(const ComparableRange& other) const;
        bool operator==(const ComparableRange& other) const;
    private:
        OffsetsRange *range;
    friend class ByteSourceAbstract;
};

class ByteSourceAbstract : public QObject
{
        Q_OBJECT
    public:
        enum CAPABILITIES {
            CAP_RESET = 0x01,
            CAP_COMPARE = 0x02,
            CAP_RESIZE = 0x04,
            CAP_WRITE = 0x08,
            CAP_HISTORY = 0x10,
            CAP_TRANSFORM = 0x20,
            CAP_LOADFILE = 0x40,
            CAP_SEARCH = 0x80
        };

        struct Markings {
                QColor bgcolor;
                QColor fgcolor;
                QString text;
                bool operator==(const Markings& other) const {
                    return (bgcolor == other.bgcolor) && (fgcolor == other.fgcolor) && (text == other.text);
                }
        };

        explicit ByteSourceAbstract(QObject *parent = 0);
        virtual ~ByteSourceAbstract();
        virtual QString description() = 0;
        virtual QString name() = 0;
        virtual void setData(QByteArray data, quintptr source = INVALID_SOURCE); // not always possible
        virtual QByteArray getRawData(); // not always possible
        virtual quint64 size();
        virtual int viewSize();
        virtual QByteArray extract(quint64 offset, int length) = 0;
        virtual QByteArray viewExtract(int offset, int length);
        virtual char extract(quint64 offset) = 0;
        virtual char viewExtract(int offset);
        virtual void replace(quint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void viewReplace(int offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void insert(quint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void viewInsert(int offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        virtual void remove(quint64 offset, int length, quintptr source = INVALID_SOURCE);
        virtual void viewRemove(int offset, int length, quintptr source = INVALID_SOURCE);
        virtual void clear(quintptr source = INVALID_SOURCE);

        virtual void fromLocalFile(QString fileName);
        virtual void saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset);
        virtual void saveToFile(QString destFilename);

        virtual int getViewOffset(quint64 realoffset);
        virtual quint64 getRealOffset(int viewOffset);
        virtual quint64 startingRealOffset();
        virtual bool isOffsetValid(quint64 offset);
        virtual bool isReadableText();

        SearchAbstract * getSearchObject(QObject *parent = 0,bool singleton = true);
        virtual bool tryMoveUp(int size);
        virtual bool tryMoveDown(int size);
        virtual bool hasDiscreetView();

        QWidget * getGui(QWidget *parent = 0);

        virtual void viewMark(int start, int end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());

        virtual void viewClearMarking(int start, int end);
        void clearMarking(quint64 start, quint64 end);
        bool hasMarking() const;
        virtual QColor getBgColor(quint64 pos);
        virtual QColor getBgViewColor(int pos);
        virtual QColor getFgColor(quint64 pos);
        virtual QColor getFgViewColor(int pos);
        virtual QString getToolTip(quint64 pos);
        virtual QString getViewToolTip(int pos);

        bool hasCapability(CAPABILITIES cap);
        quint32 getCapabilities() const;
        virtual bool setReadOnly(bool readonly = true);
        virtual bool isReadonly();

        virtual int preferredTabType();
        static QString toPrintableString(const QByteArray &val);

        virtual quint64 lowByte();
        virtual quint64 highByte();
        virtual int textOffsetSize();

    public slots:
        virtual bool historyForward();
        virtual bool historyBackward();
        virtual void historyClear();
        virtual void setViewSize(int size);
        void clearAllMarkings();
        void mark(quint64 start, quint64 end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());
        // use the next function with caution as it does not send updates signals
        void markNoUpdate(quint64 start, quint64 end, const QColor &bgcolor,const QColor &fgColor = QColor(), QString toolTip = QString());
    private slots:
        void onGuiDestroyed();
    signals:
        void updated(quintptr source);
        void minorUpdate(quint64,quint64);
        void log(QString mess, QString source, Pip3lineConst::LOGLEVEL level);
        void nameChanged(QString newName);
        void sizeChanged();
        
    protected:
        enum HistAction {INSERT = 0, REMOVE = 1, REPLACE = 2}; // int values are used for debugging only
        struct HistItem {
               HistAction action;
               quint64 offset;
               QByteArray before;
               QByteArray after;
        };
        QList<HistItem> history;
        int currentHistoryPointer;
        bool applyingHistory;

        enum TabType {TAB_GENERIC = 0, TAB_TRANSFORM = 1, TAB_LARGERANDOM = 2};
        virtual QWidget * requestGui(QWidget *parent);
        virtual SearchAbstract *requestSearchObject(QObject *parent);
        void historyApply(HistItem item, bool forward);
        void historyAddInsert(quint64 offset, QByteArray after);
        void historyAddRemove(quint64 offset, QByteArray before);
        void historyAddReplace(quint64 offset, QByteArray before, QByteArray after);
        void historyAdd(HistItem item);
        void writeToFile(QString destFilename, QByteArray data);
        void clearAllMarkingsNoUpdate();
        bool _readonly;
        static const quintptr INVALID_SOURCE;
        quint32 capabilities;

        QMap<ComparableRange, Markings> userMarkingsRanges;
        OffsetsRange *cachedMarkingRange;
        QWidget *confGui;
        SearchAbstract *searchObj;
};



#endif // BYTESOURCEABSTRACT_H
