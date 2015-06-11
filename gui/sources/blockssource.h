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
#include "commonstrings.h"

class QWidget;
class BlocksSource;

class Block {
    public:
        explicit Block();

        enum BLOCK_ORIGIN {
            SOURCE,
            TARGET
        };
        QByteArray data;
        BlocksSource * source;
        BLOCK_ORIGIN direction;
        int sourceid;
};

class BlocksSource : public QObject
{
        Q_OBJECT
    public:
        explicit BlocksSource(QObject *parent = 0);
        virtual ~BlocksSource();
        QWidget *getGui(QWidget * parent = 0);
        bool isReadWrite();
        static const int BLOCK_MAX_SIZE;
        char getSeparator() const;
        void setSeparator(char value);
        virtual void postBlockForSending(Block block);
        bool getEncodeOutput() const;
        void setEncodeOutput(bool value);
        bool getDecodeinput() const;
        void setDecodeinput(bool value);

    signals:
        void blockReceived(Block block);
        void blockToBeSend(Block block);
        void stopped();
        void started();
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
    public slots:
        virtual void sendBlock(const Block & block);
        virtual bool startListening() = 0;
        virtual void stopListening() = 0;
        virtual void restart();
    protected:
        virtual QWidget *requestGui(QWidget * parent);
        bool readWrite;
        char separator;
        bool encodeOutput;
        bool decodeInput;
    private slots:
        void onGuiDestroyed();
    private:
        QWidget * gui;
};

#endif // BLOCKSSOURCE_H
