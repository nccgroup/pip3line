/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CLEARALLMARKINGSBUTTON_H
#define CLEARALLMARKINGSBUTTON_H

#include <QPushButton>
class ByteSourceAbstract;

class ClearAllMarkingsButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit ClearAllMarkingsButton(ByteSourceAbstract *bytesource, QWidget *parent = 0);

    private:
        ByteSourceAbstract *byteSource;

};

#endif // CLEARALLMARKINGSBUTTON_H
