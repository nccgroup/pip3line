/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifdef PCAPSOURCE_H
#define PCAPSOURCE_H

#include <commonstrings.h>
#include <pcap.h>
#include "bytesourceabstract.h"

class GuiHelper;

class PcapSource : public ByteSourceAbstract
{
        Q_OBJECT
    public:
        explicit PcapSource(QObject *parent = 0);
        ~PcapSource();
        QString description();
        QByteArray getRawData();
        quint64 size();
        QByteArray extract(quint64 offset, int length);
        char extract(quint64 offset);
        void replace(quint64, int, QByteArray, quintptr = INVALID_SOURCE);
        void insert(quint64 , QByteArray , quintptr source = INVALID_SOURCE);
        void remove(quint64 , int , quintptr source = INVALID_SOURCE);
        void clear(quintptr = INVALID_SOURCE);
        bool historyForward();
        bool historyBackward();
        void fromLocalFile(QString fileName);
        bool isReadableText();
        
    private:
        Q_DISABLE_COPY(PcapSource)
        QByteArray currentData;
        static const QString LOGID;
        bool validateOffsetAndSize(quint64 offset, int length);
        pcap_t *currentFile;
};

#endif // PCAPSOURCE_H
