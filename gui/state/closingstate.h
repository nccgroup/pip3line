/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CLOSINGSTATE_H
#define CLOSINGSTATE_H

#include "basestateabstract.h"

/**
 * @brief The ClosingState class
 *
 * Utility class used to perform close actions such as closing group
 * or array open on the state file, or clearing the state file object
 *
 */

class ClosingState : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit ClosingState();
        virtual ~ClosingState();
        virtual void run();
};

#endif // CLOSINGSTATE_H
