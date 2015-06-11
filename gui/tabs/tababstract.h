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
#include <QSettings>
#include "shared/guiconst.h"
#include "state/basestateabstract.h"

class GuiHelper;
class LoggerWidget;
class ByteSourceAbstract;
class ByteTableView;
class QSettings;
class QState;

class TabAbstract : public QWidget
{
        Q_OBJECT
    public:
        static const int WINDOWED_TAB;
        explicit TabAbstract(GuiHelper *guiHelper , QWidget *parent = 0);
        virtual ~TabAbstract();
        virtual QString getName() const ;
        virtual void bringFront();
        virtual void loadFromFile(QString fileName) = 0;
        virtual int getBlockCount() const = 0;
        virtual ByteSourceAbstract *getSource(int blockIndex) = 0;
        virtual ByteTableView *getHexTableView(int blockIndex) = 0;
        virtual void setData(const QByteArray &data) = 0;
        virtual bool canReceiveData();
        GuiConst::AVAILABLE_PRETABS getPreTabType() const;
        void setPreTabType(const GuiConst::AVAILABLE_PRETABS &value);
        virtual BaseStateAbstract *getStateMngtObj() = 0;
        GuiHelper * getHelper();

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
        QString name;
        GuiConst::AVAILABLE_PRETABS preTabType;
    private:
        Q_DISABLE_COPY(TabAbstract)
};

class TabStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit TabStateObj(TabAbstract *tab);
        virtual ~TabStateObj();
        virtual void run();
        bool getIsWindowed() const;
        void setIsWindowed(bool value);
        QByteArray getWindowState() const;
        void setWindowState(const QByteArray &value);

    protected:
        TabAbstract *tab;
        bool isWindowed;
        QByteArray windowState;
};

#endif // TABABSTRACT_H
