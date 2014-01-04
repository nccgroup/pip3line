/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SINGLEVIEWABSTRACT_H
#define SINGLEVIEWABSTRACT_H

#include <QWidget>

class ByteSourceAbstract;
class GuiHelper;
class LoggerWidget;

class SingleViewAbstract : public QWidget
{
        Q_OBJECT
    public:
        explicit SingleViewAbstract(ByteSourceAbstract *dataModel,GuiHelper *guiHelper, QWidget *parent = 0);
        virtual ~SingleViewAbstract();
    public slots:
        virtual void search(QByteArray item) = 0;
        virtual void searchAgain();
    signals:
        void askForFileLoad();
    protected:
        ByteSourceAbstract *byteSource;
        GuiHelper * guiHelper;
        LoggerWidget *logger;
        QByteArray previousSearch;
        
};

#endif // SINGLEVIEWABSTRACT_H
