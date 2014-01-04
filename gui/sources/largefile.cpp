/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "largefile.h"
#include "filewidget.h"
#include "../crossplatform.h"
#include <QDebug>
#include <string.h>

const int FileSearch::STATSBLOCK = 4096;
const int FileSearch::BufferSize = 4096;

FileSearch::FileSearch(QString fileName, QObject *parent) : SearchAbstract(parent), filename(fileName)
{
    singleSearch = true;
    stopped = false;
}

FileSearch::~FileSearch()
{

}

void FileSearch::internalStart()
{
  //  qDebug() << "initial params" << soffset << eoffset;
    if (soffset > LONG_LONG_MAX || eoffset > LONG_LONG_MAX) {
        emit log(tr("Hitting the LONG_LONG_MAX file offset limit for Qt, ignoring"),this->metaObject()->className(), Pip3lineConst::LERROR);
        emit errorStatus(true);
        return;
    }
    int searchSize = sitem.size();
    if (searchSize <= 0) {
        emit log(tr("Empty search Item, ignoring"),this->metaObject()->className(), Pip3lineConst::LERROR);
        return;
    }
    if (eoffset > (quint64)(LONG_LONG_MAX - searchSize)) {
        emit log(tr("End search offset is overflowing, ignoring"),this->metaObject()->className(), Pip3lineConst::LERROR);
        emit errorStatus(true);
        return;
    }

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        emit log(tr("Failed to open %1:\n %2").arg(filename).arg(file.errorString()),this->metaObject()->className(),Pip3lineConst::LERROR);
        emit errorStatus(true);
        return;
    }

    qint64 fileSize = file.size();

    if (fileSize < searchSize) {
        emit log(tr("File size is inferior to the item size"),this->metaObject()->className(), Pip3lineConst::LERROR);
        emit errorStatus(true);
        file.close();
        return;
    }
    qint64 curOffset = (qint64)soffset;
    qint64 eOffset = eoffset;
    if (curOffset >= eOffset || eOffset > fileSize - searchSize) { // setting to the end for both case
        eOffset = fileSize - searchSize;
    } else {
        eOffset = eoffset - searchSize; // no need to
    }
    bool looping = singleSearch;
    if (curOffset > fileSize - searchSize) { // if we are at the end of the file and there is not enough data to compare, go back to beginning
        if (singleSearch)
            curOffset = 0;
        else {
            emit errorStatus(true);
            file.close();
            return;
        }

    }


    if (!file.seek(curOffset)) {
        emit log(tr("Error while seeking: %1").arg(file.errorString()),this->metaObject()->className(), Pip3lineConst::LERROR);
        file.close();
        emit errorStatus(true);
        return;
    }

    //qDebug() << "from" << curOffset << eOffset;

    statsSize = (quint64)file.size();
    char * readbuffer = new(std::nothrow) char[BufferSize];
    if (readbuffer == NULL) {
        qFatal("Cannot allocate memory for the wind buffer X{");
        return;
    }

    char * mask = new(std::nothrow) char[searchSize];
    if (readbuffer == NULL) {
        qFatal("Cannot allocate memory for the wind buffer X{");
        return;
    }

    for (int i = 0; i < searchSize;i++) {
        mask[i] = '\xFF';
    }

    bool found = fastSearch(&file,readbuffer,mask,searchSize,curOffset, eOffset);
    if (!found && looping && curOffset + searchSize + 1 < fileSize) // trying again if looping is authorized
    {
        found = fastSearch(&file,readbuffer,mask,searchSize,0, curOffset + searchSize + 1);
    }

    emit errorStatus(!found);


    delete [] readbuffer;
    delete [] mask;
    file.close();
 //   qDebug() << "End of file search";
}

void FileSearch::internalThreadedStart()
{
    if (sitem.isEmpty())
        return;
    QFileInfo info(filename);
    SearchAbstract * sobj = NULL;
    quint64 fsize = info.size();
    qDebug() << "Source size" << fsize;
    quint64 blocksize = fsize / (quint64)threadCount;
    timer = startTimer(400);
    int i = 0;
    for (i = 0; i < threadCount - 1; i++) {
        sobj = new (std::nothrow) FileSearch(info.absoluteFilePath());
        if (sobj != NULL) {
            registerSearchObject(sobj);
            sobj->setStartOffset(i * blocksize);
            sobj->setEndOffset((i+1) * blocksize + sitem.size());
            QTimer::singleShot(0,sobj,SLOT(startSearch()));
        }
    }
    sobj = new (std::nothrow) FileSearch(info.absoluteFilePath());
    if (sobj != NULL) {
        registerSearchObject(sobj);
        sobj->setStartOffset(i * blocksize);
        sobj->setEndOffset(fsize);
        QTimer::singleShot(0,sobj,SLOT(startSearch()));
    }
}

bool FileSearch::fastSearch(QFile *file, char *readbuffer, char *mask, qint64 searchSize, qint64 startOffset, qint64 endOffset)
{
    bool found = false;
    if (startOffset == endOffset)
        return true;

    qint64 cursorTravel= 0;
    qint64 nextStat = STATSBLOCK;
    int len = searchSize -1;
    char * value = sitem.data();
    qint64 curOffset = startOffset;

    if (!file->seek(curOffset)) {
        emit log(tr("Error while seeking: %1").arg(file->errorString()),this->metaObject()->className(), Pip3lineConst::LERROR);
        return found;
    }

    while (true)
    {
        qint64 bRead;
        // Visual Studio is going to complain here because I am using memmove ...
        memmove(readbuffer, readbuffer + BufferSize - len, len);
        bRead = file->read(readbuffer + len, BufferSize - len);

        if (bRead < 0)
        {
            emit log(tr("Error while reading the file: %1").arg(file->errorString()),this->metaObject()->className(), Pip3lineConst::LERROR);
            return found;
        } else if (bRead == 0) // end of file ... hopefully
            return found;

        int off, i;
        for (off = curOffset ? 0 : len; off < bRead; ++off)
        {
            for (i = 0; i <= len; ++i)
                if ((readbuffer[off + i] & mask[i]) != value[i])
                        break;
            if (i > len)
            {
                emit itemFound(curOffset + (quint64)off - (quint64)len,curOffset + (quint64)off);
             //   qDebug() << "found" << curOffset;
                if (!found) {
                    if (singleSearch)
                        return true;
                    else
                        found = true;
                }
            }
            // updating stats
            cursorTravel++;
            if (cursorTravel > nextStat) {
                if (processStatsInternally)
                    emit progressStatus((double) cursorTravel / (double) statsSize);
                else
                    emit progressUpdate((quint64)cursorTravel);

                nextStat += STATSBLOCK;
            }

            if (startOffset + cursorTravel > endOffset) // end of the travel
                return found;
        }

        curOffset += bRead;
        stopMutex.lock();
        if (stopped) {
            stopMutex.unlock();
            break;
        }
        stopMutex.unlock();
    }

    return found;
}

const int LargeFile::BLOCKSIZE = 1024;
const qint64 LargeFile::MAX_COMPARABLE_SIZE = 0x8000000;

LargeFile::LargeFile(QObject *parent) :
    LargeRandomAccessSource(parent)
{
    capabilities = CAP_LOADFILE;
    chunksize = BLOCKSIZE;
    fileSize = 0;
    connect(&watcher, SIGNAL(fileChanged(QString)), SLOT(onFileChanged(QString)));
}

LargeFile::~LargeFile()
{
    file.close();
}

QString LargeFile::description()
{
    return tr("File: %1").arg(file.fileName());
}

QString LargeFile::name()
{
    return file.fileName();
}

quint64 LargeFile::size()
{
    if (isFileReadable()) {
        return (quint64)fileSize; // this should be a non-negative value
    }
    return 0;
}

void LargeFile::fromLocalFile(QString fileName)
{
    fromLocalFile(fileName, 0);
}

void LargeFile::fromLocalFile(QString fileName, quint64 startOffset)
{
    if (file.isOpen()) {
        watcher.removePath(infoFile.absoluteFilePath());
        file.close();
    }

    historyClear();
    capabilities = CAP_LOADFILE;
    file.setFileName(fileName);
    infoFile.setFile(file);

    if (!infoFile.isReadable()) {
        emit log(tr("File %1 is not readable").arg(fileName),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }

    if (infoFile.isWritable()) {
        emit log(tr("File %1 is writeable").arg(fileName),metaObject()->className(),Pip3lineConst::LSTATUS);

        if (!file.open(QIODevice::ReadWrite)) {
            emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
            _readonly = true;
        } else {
            capabilities = capabilities | CAP_HISTORY | CAP_SEARCH | CAP_WRITE;
            _readonly = false;
        }
    } else {
        emit log(tr("File %1 is not writeable").arg(fileName),metaObject()->className(),Pip3lineConst::LSTATUS);
        _readonly = true;
        if (!file.open(QIODevice::ReadOnly)) {
            emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
        } else {
            capabilities = capabilities | CAP_SEARCH;
        }
    }

    if (file.isOpen()) {
        fileSize = infoFile.size();
        if (fileSize < 0) {
            emit log(tr("File size is negative, setting to zero"),metaObject()->className(),Pip3lineConst::LERROR);
            fileSize = 0;
        }
        if (fileSize < MAX_COMPARABLE_SIZE) {
            capabilities = capabilities | CAP_COMPARE;
        } else {
            emit log(tr("File is too large to be used as a comparable sample in its entirety. Selection comparison will still work, however."),metaObject()->className(),Pip3lineConst::LWARNING);
        }
        refreshData(false);
        // file watcher
        watcher.addPath(infoFile.absoluteFilePath());
        emit nameChanged(infoFile.fileName());
    }
    currentStartingOffset = startOffset;

    emit infoUpdated();

    emit updated(INVALID_SOURCE);
    emit sizeChanged();
}

QString LargeFile::fileName() const
{
    return (file.isReadable() ? infoFile.absoluteFilePath() : "INVALID");
}

void LargeFile::saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset)
{
    if (fileSize < 1)
        return;

    ByteSourceAbstract::saveToFile(destFilename, startOffset, endOffset);
}

void LargeFile::saveToFile(QString destFilename)
{
    if (!destFilename.isEmpty()) {
        QString sfileName = fileName();
        QFile sfile(sfileName);
        if (!sfile.copy(destFilename)) {
            emit log(tr("Failed to copy %1 to %2:\n %3").arg(sfileName).arg(destFilename).arg(sfile.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
        }
    } else {
        emit log(tr("Destination file is empty, ignoring save."),metaObject()->className(),Pip3lineConst::LERROR);
    }
}

bool LargeFile::isOffsetValid(quint64 offset)
{
    return offset < (quint64)fileSize; //  filesize should always be positive
}

bool LargeFile::tryMoveUp(int sizeToMove)
{
    return tryMoveView(-sizeToMove);
}

bool LargeFile::tryMoveDown(int sizeToMove)
{
    return tryMoveView(sizeToMove);
}

bool LargeFile::tryMoveView(int sizeToMove)
{
    quint64 newOffset;

    if (sizeToMove < 0) {
        if (currentStartingOffset == 0)
            return false; // already at the beginning, nothing to see here

        if (currentStartingOffset < (quint64)(-1 * sizeToMove)) { // checking how much we can go up
            newOffset = 0;
        } else {
            newOffset = currentStartingOffset + sizeToMove;
        }
    } else {
        if (ULONG_LONG_MAX - (quint64)sizeToMove - (quint64)chunksize < currentStartingOffset) {
            return false; // checking overflow
        }

        if (currentStartingOffset + (quint64)chunksize >= (quint64)fileSize) {
            return false; // no more data
        }

        newOffset = currentStartingOffset + sizeToMove;
    }

    int readsize = qMin((quint64)fileSize - newOffset,(quint64)chunksize);
    QByteArray temp;
    if (!readData(newOffset,temp,readsize)) {
        return false;
    }
    currentStartingOffset = newOffset;
    dataChunk = temp;
    emit updated(INVALID_SOURCE);
    emit sizeChanged();
    return true;
}

void LargeFile::onFileChanged(QString path)
{
    qDebug() << "File updated externally: " << path;
    fromLocalFile(file.fileName(),currentStartingOffset);
}

bool LargeFile::isFileReadable()
{
    if (!file.isReadable()) {
        emit log(tr("File %1 is not readable").arg(file.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

bool LargeFile::isFileWriteable()
{
    if (!file.isWritable()) {
        emit log(tr("File %1 is not writable").arg(file.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

bool LargeFile::seekFile(quint64 offset)
{
    if (offset > LONG_LONG_MAX) {
        emit log(tr("[seekFile] Hitting the LONG_LONG_MAX limit for Qt, ignoring"),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    if (!file.seek(offset)) {
        emit log(tr("Error while seeking: %1").arg(file.errorString()),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    return true;
}

QWidget *LargeFile::requestGui(QWidget *parent)
{
    FileWidget *fw = new(std::nothrow)FileWidget(this,parent);
    if (fw == NULL) {
        qFatal("Cannot allocate memory for FileWidget X{");
    }
    return fw;
}

SearchAbstract *LargeFile::requestSearchObject(QObject *parent)
{
    FileSearch *sObj = new(std::nothrow) FileSearch(infoFile.absoluteFilePath(), parent);

    if (sObj == NULL) {
        qFatal("Cannot allocate memory for FileSearch X{");
    }

    return sObj;
}

bool LargeFile::readData(quint64 offset, QByteArray &data, int length)
{
    data.clear();
   // qDebug() << "read Data: " << offset << length << fileSize;
    bool noError = true;
    if (length < 1 || offset > (quint64)fileSize)
        noError = false;
    else if (isFileReadable()) {

        if ((quint64)(LONG_LONG_MAX - length) < offset ) {
            qDebug() << tr("Hitting LONG_LONG_MAX limit");
            noError = false;
        } else if (seekFile(offset)) {
            char * buf = new(std::nothrow) char[length];
            if (buf == NULL) {
                qFatal("Cannot allocate memory for the file buffer X{");
            }
            if (offset + (quint64)length > (quint64)fileSize) {
                length = (quint64)fileSize - offset; // just skip the last bytes if the requested block goes out-of-bound
                qDebug() << "Reducing length";
            }
         //   qDebug() << "read Data(2): " << offset << length << fileSize;
            qint64 bytesRead = file.read(buf, length);
            if (bytesRead < 0) {
                emit log(tr("Cannot read file: %1").arg(file.errorString()), metaObject()->className(), Pip3lineConst::LERROR);
                noError = false;
            } else if (bytesRead == 0) {
                qDebug() << tr("No byte returned when reading file");
                noError = false;
            } else {
                if (bytesRead < length) {
                    qDebug() << tr("Length read (%1) inferior to length requested (%2) at %3").arg(bytesRead).arg(length).arg(offset);
                }
                data = QByteArray(buf, bytesRead);
            }

            delete [] buf;
        }
    }
    return noError;
}

bool LargeFile::writeData(quint64 offset, int length, const QByteArray &repData, quintptr source)
{
    bool noError = false;
    if (_readonly) return noError;

    if (fileSize < 0 ) {
        emit log(tr("File size is negative, X["), metaObject()->className(), Pip3lineConst::LERROR);
        return noError;
    }

    if (length < 0 )  {
        emit log(tr("length is negative, X["), metaObject()->className(), Pip3lineConst::LERROR);
        return noError;
    }

    if (isFileWriteable()) {

        if (offset > (quint64)fileSize) {
            emit log(tr("Offset is outside the file, ignoring"), metaObject()->className(), Pip3lineConst::LERROR);
            return noError;
        }

        if (repData.size() < length){
            emit log(tr("replace: target block length is superior to the new block length, only overwriting the first bytes"), metaObject()->className(), Pip3lineConst::LWARNING);
        }

        if (repData.size() > length){
            emit log(tr("replace: target block length is inferior to the new block length, overwriting the next bytes (without resizing the file)"), metaObject()->className(), Pip3lineConst::LWARNING);
            length = repData.size();
        }

        if (length > fileSize) { // stupidity check
            emit log(tr("Replace length is greater than the file size, ignoring"), metaObject()->className(), Pip3lineConst::LERROR);
            return noError;
        }

        if ((quint64) (fileSize - length) < offset) { // stupidity check
            emit log(tr("Offset + length is outside the file, ignoring"), metaObject()->className(), Pip3lineConst::LERROR);
            return noError;
        }

        QByteArray before = extract(offset,repData.size());

        if (seekFile(offset)) {
            watcher.removePath(infoFile.absoluteFilePath());
            qint64 bytesWritten = file.write(repData);
            file.flush();
            if (bytesWritten < 0) {
                emit log(tr("Cannot write to file: %1").arg(file.errorString()), metaObject()->className(), Pip3lineConst::LERROR);
            } else if (bytesWritten == 0) {
                emit log(tr("No byte written to the file"), metaObject()->className(), Pip3lineConst::LWARNING);
            } else {
                noError = true;
                if (bytesWritten < repData.size()) {
                    emit log(tr("Was not able to write all the bytes to the file"), metaObject()->className(), Pip3lineConst::LWARNING);
                }
                historyAddReplace(offset,before,repData);
                emit updated(source);
            }
            watcher.addPath(infoFile.absoluteFilePath());
        }
    }

    return noError;
}

