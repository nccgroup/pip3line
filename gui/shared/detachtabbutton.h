/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DETACHTABBUTTON_H
#define DETACHTABBUTTON_H

#include <QPushButton>
class TabAbstract;

class DetachTabButton : public QPushButton
{
        Q_OBJECT
    public:
        explicit DetachTabButton(TabAbstract *tab);
    private:
        TabAbstract * curtab;

};

#endif // DETACHTABBUTTON_H
