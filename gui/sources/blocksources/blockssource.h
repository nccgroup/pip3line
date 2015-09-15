/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BLOCKSSOURCE_H
#define BLOCKSSOURCE_H

#include <QThread>
#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <QHash>
#include "commonstrings.h"

class QWidget;
class BlocksSource;

class Block {
    public:
        explicit Block(QByteArray data, int sourceid);
        ~Block();
        QByteArray getData() const;
        void setData(const QByteArray &value);
        int getSourceid() const;
        void setSourceid(int value);
    private:
        QByteArray data;
        int sourceid;
};

class BlocksSource : public QObject
{
        Q_OBJECT
    public:
        enum BSOURCETYPE {CLIENT          = 0x0001,
                          SERVER          = 0x0002,
                          SNIFFER         = 0x0004,
                          EXTERNAL_SERVER = 0x0008,
                          EXTERNAL_CLIENT = 0x0010,
                          INVALID_TYPE    = 0xFFFF};
        explicit BlocksSource(QObject *parent = 0);
        virtual ~BlocksSource();
        QWidget *getGui(QWidget * parent = 0);
        bool isReadWrite();
        static const int BLOCK_MAX_SIZE;
        char getSeparator() const;
        void setSeparator(char value);
        virtual void postBlockForSending(Block *block);
        bool getEncodeOutput() const;
        void setEncodeOutput(bool value);
        bool getDecodeinput() const;
        void setDecodeinput(bool value);
        static int newSourceID(BlocksSource * source = NULL);
        static void releaseID(int id);
        static BlocksSource * getSourceObject(int id);
        BSOURCETYPE getType() const;
        virtual QString getName() = 0;
        virtual bool isReflexive() = 0;
    signals:
        void blockReceived(Block *block);
        void blockToBeSend(Block *block);
        void stopped();
        void started();
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
    public slots:
        virtual void sendBlock(Block * block) = 0;
        virtual bool startListening() = 0;
        virtual void stopListening() = 0;
        virtual void restart();
    protected:
        virtual QWidget *requestGui(QWidget * parent);
        bool readWrite;
        char separator;
        bool encodeOutput;
        bool decodeInput;
        int sid;
        BSOURCETYPE type;
    private slots:
        void onGuiDestroyed();
    private:
        QWidget * gui;
        static int currentid;
        static QHash<int,BlocksSource *> idSourceTable;
        static QMutex idlock;
};

#endif // BLOCKSSOURCE_H
