#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(CIRCLEBUFFER_LIB)
#  define CIRCLEBUFFER_EXPORT Q_DECL_EXPORT
# else
#  define CIRCLEBUFFER_EXPORT Q_DECL_IMPORT
# endif
#else
# define CIRCLEBUFFER_EXPORT
#endif
