/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LARGEFILE_H
#define LARGEFILE_H

#include "largerandomaccesssource.h"
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMutex>

class FileSearch : public SearchAbstract {
    Q_OBJECT
    public:
        FileSearch(QString fileName, QObject *parent = 0);
        ~FileSearch();
    private:
        void internalStart();
        void internalThreadedStart();
        bool fastSearch(QFile * file, char * readbuffer, char * mask, qint64 searchSize, qint64 startOffset, qint64 endOffset);
        static const int STATSBLOCK;
        QString filename;
        static const int BufferSize;
};

class LargeFile : public LargeRandomAccessSource
{
        Q_OBJECT
    public:
        explicit LargeFile(QObject *parent = 0);
        ~LargeFile();
        QString description();
        QString name();
        quint64 size();
        void fromLocalFile(QString fileName);
        void fromLocalFile(QString fileName,quint64 startOffset);
        QString fileName() const;
        void saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset);
        void saveToFile(QString destFilename);
        bool isOffsetValid(quint64 offset);
        bool tryMoveUp(int size);
        bool tryMoveDown(int size);
        bool tryMoveView(int size);
    signals:
        void infoUpdated();
    private slots:
        void onFileChanged(QString path);
    private:
        Q_DISABLE_COPY(LargeFile)
        static const int BLOCKSIZE;
        static const qint64 MAX_COMPARABLE_SIZE;
        bool isFileReadable();
        bool isFileWriteable();
        bool seekFile(quint64 offset);
        QWidget * requestGui(QWidget *parent);
        SearchAbstract *requestSearchObject(QObject *parent = 0);
        bool readData(quint64 offset, QByteArray &data, int size);
        bool writeData(quint64 offset, int length, const QByteArray &data, quintptr source);
        QFile file;
        qint64 fileSize;
        QFileInfo infoFile;
        QFileSystemWatcher watcher;

};

#endif // LARGEFILE_H
