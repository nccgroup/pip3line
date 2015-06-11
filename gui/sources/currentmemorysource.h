/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/


#ifndef CURRENTMEMORYSOURCE_H
#define CURRENTMEMORYSOURCE_H


#include "largerandomaccesssource.h"
#include <QAbstractListModel>
#include <QStringList>

class CurrentMemorysource;

// extending OffsetRange to add functionalities specific to memory management
class MemRange : public BytesRange {
    public:
        explicit MemRange(quint64 lowerVal, quint64 upperVal, QString description = QString());
        ~MemRange() {}
        void setExec(bool val);
        bool isExec() const;
        void setRead(bool val);
        bool isRead() const;
        void setWrite(bool val);
        bool isWrite() const;
        void setPriv(bool val);
        bool isPriv() const;
        QString toString();
#if defined(Q_OS_WIN)
        bool getCopy() const;
        void setCopy(bool value);
#endif
        bool operator<(const MemRange& other) const;
    private:
        bool read;
        bool write;
        bool exec;
#if defined(Q_OS_WIN)
        bool copy;
#endif
        bool priv;
};

class MemRangeModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        explicit MemRangeModel(QObject * parent = 0);
        ~MemRangeModel();
        QList<MemRange *> getList();
        bool isOffsetInRange(quint64 offset);
        MemRange * getRange(quint64 offset);
        MemRange * getRange(const QModelIndex &index);
        void setCurrentMem(const QModelIndex & parent = QModelIndex());
        int rowCount(const QModelIndex & parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    public slots:
        void clear();
        void addRange(MemRange *range);
        void setCurrentRange(MemRange *range);
    private:
        enum COLUMN { START_OFFSET = 0,
                      END_OFFSET = 1,
                      PERMISSIONS = 2,
                      SIZE = 3,
                      DESCRIPTION = 4}; // don't change the numbering, it is in sync with the QStringlist
        static const QFont RegularFont;
        QList<MemRange *> ranges;
        int currentMemRow;
        static const QStringList headers;
};

class MemSearch : public SearchAbstract {
    Q_OBJECT
    public:
        explicit MemSearch(CurrentMemorysource * source);
        ~MemSearch();
    private:
        void internalStart();
        QList<MemRange *> ranges;
};

class MemSourceReader : public SourceReader
{
    public:
        explicit MemSourceReader(QString filename);
        ~MemSourceReader();
        bool seek(quint64 pos);
        int read(char * buffer, int maxLen);
        bool isReadable();
    private:
        QList<MemRange *> allocatedRanges;
};

class CurrentMemorysource : public LargeRandomAccessSource
{
        Q_OBJECT
    public:
        explicit CurrentMemorysource(QObject *parent = 0);
        ~CurrentMemorysource();
        QString description();
        quint64 size();
        bool isOffsetValid(quint64 offset);
        MemRangeModel * getMemRangesModel() const;
        void mapMemory();
        quint64 lowByte();
        quint64 highByte();
        bool tryMoveUp(int size);
        bool tryMoveDown(int size);
        bool tryMoveView(int size);
        void fromLocalFile(QString fileName);

    public slots:
        bool setStartingOffset(quint64 offset);
    signals:
        void mappingChanged();
    private:
        QWidget *requestGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type);
        bool readData(quint64 offset, QByteArray &data, int size);
        bool writeData(quint64 offset, int length, const QByteArray &data, quintptr source);
        MemRangeModel *rangesModel;
        QString errorString(int errnoVal);
        MemRange *currentRange;
};

#endif // CURRENTMEMORYSOURCE_H
