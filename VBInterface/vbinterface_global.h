#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(VBINTERFACE_LIB)
#  define VBINTERFACE_EXPORT Q_DECL_EXPORT
# else
#  define VBINTERFACE_EXPORT Q_DECL_IMPORT
# endif
#else
# define VBINTERFACE_EXPORT
#endif
