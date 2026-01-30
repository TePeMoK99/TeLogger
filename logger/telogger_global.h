#ifndef TELOGGER_GLOBAL_H
#define TELOGGER_GLOBAL_H

#include <QtGlobal>

#if defined(Q_OS_WIN)
#if defined(TELOGGER_LIB)
#define TELOGGER_EXPORT Q_DECL_EXPORT
#elif
#define TELOGGER_EXPORT Q_DECL_IMPORT
#else
#define TELOGGER_EXPORT
#endif

#endif // TELOGGER_GLOBAL_H
