/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef LIBTRANSFORM_GLOBAL_H
#define LIBTRANSFORM_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBTRANSFORM_LIBRARY)
#  define LIBTRANSFORMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBTRANSFORMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBTRANSFORM_GLOBAL_H
