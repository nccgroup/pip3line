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
class QWidget;

class BlocksSource : public QObject
{
        Q_OBJECT
    public:
        explicit BlocksSource(QObject *parent = 0);
        virtual ~BlocksSource();
        QWidget *getGui(QWidget * parent = 0);
        bool isReadWrite();
        static const int BLOCK_MAX_SIZE;
        bool getBase64Applied() const;
        void setBase64Applied(bool value);
        char getSeparator() const;
        void setSeparator(char value);
    signals:
        void blockReceived(QByteArray block);
        void error(const QString, const QString);
        void status(const QString, const QString);
    public slots:
        virtual void sendBlock(const QByteArray & block) = 0;
        virtual void startListening() = 0;
        virtual void stopListening() = 0;
    protected:
        virtual QWidget *requestGui(QWidget * parent);
        bool readWrite;
        char separator;
        bool base64Applied;
    private slots:
        void onGuiDestroyed();
    private:
        QWidget * gui;
};

#endif // BLOCKSSOURCE_H
