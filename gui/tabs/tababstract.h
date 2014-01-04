/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TABABSTRACT_H
#define TABABSTRACT_H

#include <QWidget>
#include <QPushButton>

class GuiHelper;
class LoggerWidget;
class ByteSourceAbstract;
class ByteTableView;

class TabAbstract : public QWidget
{
        Q_OBJECT
    public:
        explicit TabAbstract(GuiHelper *guiHelper ,QWidget *parent = 0);
        virtual ~TabAbstract();
        virtual QString getName() const ;
        virtual void bringFront();
        virtual void loadFromFile(QString fileName) = 0;
        virtual int getBlockCount() const = 0;
        virtual ByteSourceAbstract *getSource(int blockIndex) = 0;
        virtual ByteTableView *getHexTableView(int blockIndex) = 0;
        virtual void setData(const QByteArray &data) = 0;
        virtual bool canReceiveData();
    public slots:
        virtual void setName(const QString & name);
    protected slots:
        virtual void onDetach();

    signals:
        void nameChanged();
        void askWindowTabSwitch();
        void askBringFront();
        void entriesChanged();

    protected:
        LoggerWidget *logger;
        GuiHelper * guiHelper;
        QPushButton * readonlyPushButton;
        QString name;
    private:
        Q_DISABLE_COPY(TabAbstract)
};

#endif // TABABSTRACT_H
