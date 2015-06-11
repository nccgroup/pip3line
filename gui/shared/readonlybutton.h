/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef READONLYBUTTON_H
#define READONLYBUTTON_H

#include <QPushButton>

class ByteSourceAbstract;

class ReadOnlyButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit ReadOnlyButton(ByteSourceAbstract * bytesource, QWidget *parent = 0);
    signals:
        void logError(QString message);
    public slots:
        void refreshStateValue();
        void onToggle(bool val);

    private:
        ByteSourceAbstract *byteSource;
        static const QString ReadWrite;
        static const QString ReadOnly;
        static const QString ButtonToolTipRW;
        static const QString ButtonToolTipRO;
        static const QString CAP_WRITE_DISABLED_ToolTip;
};

#endif // READONLYBUTTON_H
