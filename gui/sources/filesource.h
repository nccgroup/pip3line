#ifndef FILESOURCE_H
#define FILESOURCE_H

#include "bytesourceabstract.h"
#include <QFile>

class FileSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        explicit FileSource(QString filename, QObject *parent = 0);
        ~FileSource();
        void setData(QByteArray data, quintptr source = INVALID_SOURCE);
        QByteArray getRawData();
        qint64 size();
        QByteArray extract(qint64 offset, int length) = 0;
        void replace(qint64 offset, int length, QByteArray repData, quintptr source = INVALID_SOURCE);
        void insert(qint64 offset, QByteArray repData, quintptr source = INVALID_SOURCE);
        void remove(quint64 offset, qint64 length, quintptr source = INVALID_SOURCE);
        void clear(quintptr source = INVALID_SOURCE);
        bool contains(char c);
        bool historyForward();
        bool historyBackward();
        void fromLocalFile(QString fileName);

        bool isReadableText();
        qint64 indexOf(QByteArray item, qint64 offset = 0);
    private:
        Q_DISABLE_COPY(FileSource)
        static const QString LOGID;
        static const int BLOCKSIZE;
        bool isFileReadable();
        bool isFileWriteable();
        bool seekFile(qint64 offset);
        QFile file;

};

#endif // FILESOURCE_H
