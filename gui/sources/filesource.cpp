/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "filesource.h"

const QString FileSource::LOGID = "FileSource";
const int FileSource::BLOCKSIZE = 4096;

FileSource::FileSource(QString fileName, QObject *parent) :
    ByteSourceAbstract(parent)
{
    capabilities = CAP_REPLACE;
    file.setFileName(fileName);

    if (!file.open(QIODevice::ReadWrite)) {
        emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),LOGID,Pip3lineConst::LERROR);
    }

}

FileSource::~FileSource()
{
    file.close();
}

void FileSource::setData(QByteArray, quintptr)
{
    emit log("Overriding entire file not authorized", LOGID, Pip3lineConst::LERROR);
}

QByteArray FileSource::getRawData()
{
    emit log("No Raw Data direct access.", LOGID, Pip3lineConst::LERROR);
    return QByteArray();
}

qint64 FileSource::size()
{
    if (isFileReadable()) {
        return file.size();
    }
    return -1;
}

QByteArray FileSource::extract(qint64 offset, int length)
{
    QByteArray ret;
    if (isFileReadable()) {

        if (offset > file.size() || (LONG_MAX - length) < offset ) {
            emit log(tr("Trying to read outside the file, ignoring"), LOGID, Pip3lineConst::LERROR);
            return ret;
        }

        file.seek(offset);
        char * buf = new char[length];

        qint64 bytesRead = file.read(buf, length);
        if (bytesRead < 0) {
            emit log(tr("Cannot read file: %1").arg(file.errorString()), LOGID, Pip3lineConst::LERROR);
        } else if (bytesRead == 0) {
            emit log(tr("No byte returned when reading file"), LOGID, Pip3lineConst::LWARNING);
        } else {
            if (bytesRead < length) {
               emit log(tr("Length read inferior to length requested"), LOGID, Pip3lineConst::LWARNING);
            }
            ret = QByteArray(buf, bytesRead);
        }

        delete [] buf;
    }
    return ret;
}

char FileSource::extract(qint64 offset)
{
    QByteArray ret = extract(offset, 1);
    if (ret.isEmpty()) // error
        return '\00';
    return ret.at(0);
}

void FileSource::replace(qint64 offset, int length, QByteArray repData, quintptr source)
{
    if (isFileWriteable()) {
        if (offset > file.size()) {
            emit log(tr("Offset is outside the file, ignoring"), LOGID, Pip3lineConst::LERROR);
            return ;
        }

        if (repData.size() < length){
            emit log(tr("replace: target block length is superior to the new data length, only overwriting the first bytes"), LOGID, Pip3lineConst::LWARNING);
        }

        if (repData.size() > length){
            emit log(tr("replace: target block length is inferior to the new data length, overwriting the next bytes (possibly expanding the file)"), LOGID, Pip3lineConst::LWARNING);
        }


        file.seek(offset);
        qint64 bytesWritten = file.write(repData);
        if (bytesWritten < 0) {
            emit log(tr("Cannot write to file: %1").arg(file.errorString()), LOGID, Pip3lineConst::LERROR);
        } else if (bytesWritten == 0) {
            emit log(tr("No byte written to the file"), LOGID, Pip3lineConst::LWARNING);
        } else {
            if (bytesWritten < repData.size()) {
                emit log(tr("Was not able to write all the bytes to the file"), LOGID, Pip3lineConst::LWARNING);
            }
            emit updated(source);
        }
    }

}

void FileSource::insert(qint64 , QByteArray , quintptr )
{
    emit log(tr("insertion not supported, ignoring"), LOGID, Pip3lineConst::LERROR);
}

void FileSource::remove(quint64 , qint64 , quintptr )
{
    emit log(tr("Remove not supported, ignoring"), LOGID, Pip3lineConst::LERROR);
}

void FileSource::clear(quintptr )
{
    emit log(tr("Clearing not supported, ignoring"), LOGID, Pip3lineConst::LERROR);
}

bool FileSource::contains(char c)
{
    if (isFileReadable()) {
        file.seek(0);
        while (!file.atEnd()) {
            QByteArray ret = file.read(BLOCKSIZE);
            if (ret.contains(c)) {
                return true;
            }
        }
    }

    return false;
}

bool FileSource::historyForward()
{
    emit log(tr("History not supported, ignoring"), LOGID, Pip3lineConst::LERROR);
    return false;
}

bool FileSource::historyBackward()
{
    emit log(tr("History not supported, ignoring"), LOGID, Pip3lineConst::LERROR);
    return false;
}

void FileSource::fromLocalFile(QString fileName)
{
    file.close();
    file.setFileName(fileName);

    if (!file.open(QIODevice::ReadWrite)) {
        emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),LOGID,Pip3lineConst::LERROR);
    }

    emit updated(INVALID_SOURCE);
}

bool FileSource::isReadableText()
{
    if (file.size() > 10000)
        return false;

    if (contains('\x00')) {
        return false;
    }

    return true;
}

qint64 FileSource::indexOf(QByteArray item, qint64 offset)
{
    qint64 ret = -1;
    if (!isFileReadable())
        return ret;

    if (offset < 0 || offset > file.size()) {
        emit log(tr("Invalid offset for searching"),LOGID, Pip3lineConst::LERROR);
        return ret;
    }

    if (item.size() > file.size()) {
        emit log(tr("Search item larger than the file, ignoring"),LOGID, Pip3lineConst::LERROR);
        return ret;
    }

    if (!seekFile(offset)) {
        return ret;
    }

    int searchSize = item.size();
    char * window = new char[searchSize];
    qint64 bytesRead = 0;
    QByteArray array;
    array.resize(searchSize);

    //filling the buffer (we already now that the file is at least bigger or equal than the item searched)

    bytesRead = file.read(array.data(),searchSize);
    if (bytesRead < searchSize) { // stating the obvious
        emit log(tr("Incomplete read from search, something is wrong."),LOGID, Pip3lineConst::LERROR);
    } else if (item.contains(array)) { // in the first bytes by any chance?
            ret = offset;
    } else {
        // not luck then, need to check the rest of the file

        while (!file.atEnd() && ret < offset) {
            if (!seekFile(++offset)) {
                break;
            }
            bytesRead = file.read(window,searchSize); // not really efficient, but much easier :)
            array.setRawData(window,bytesRead); // no real need to check if bytesRead == searchSize
            if (item.contains(array)) {
                ret = offset;
            }
        }
    }


    delete [] window;
    return ret;
}


bool FileSource::isFileReadable()
{
    if (!file.isReadable()) {
        emit log(tr("File %1 is not readable").arg(file.fileName()), LOGID, Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

bool FileSource::isFileWriteable()
{
    if (!file.isWritable()) {
        emit log(tr("File %1 is not writable").arg(file.fileName()), LOGID, Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

bool FileSource::seekFile(qint64 offset)
{
    if (!file.seek(offset)) {
        emit log(tr("Error while seeking: %1").arg(file.errorString()),LOGID, Pip3lineConst::LERROR);
        return false;
    }

    return true;
}
