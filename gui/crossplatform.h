/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CROSSPLATFORM_H
#define CROSSPLATFORM_H

#ifdef Q_OS_WIN
#define LONG_LONG_MAX _I64_MAX
#define ULONG_LONG_MAX _UI64_MAX
#endif

//#define P_INT64_MAX LONG_LONG_MAX
//#define P_UINT64_MAX ULONG_LONG_MAX
//#define P_INT32_MAX INT_MAX
//#define P_UINT32_MAX UINT_MAX

#endif // CROSSPLATFORM_H
