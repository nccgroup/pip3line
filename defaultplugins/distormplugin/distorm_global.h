/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DISTORM_GLOBAL_H
#define DISTORM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DISTORM_LIBRARY)
#  define DISTORMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DISTORMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DISTORM_GLOBAL_H
