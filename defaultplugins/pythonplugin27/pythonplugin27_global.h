/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PYTHONPLUGIN27_GLOBAL_H
#define PYTHONPLUGIN27_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PYTHONPLUGIN27_LIBRARY)
#  define PYTHONPLUGIN27SHARED_EXPORT Q_DECL_EXPORT
#else
#  define PYTHONPLUGIN27SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PYTHONPLUGIN27_GLOBAL_H
