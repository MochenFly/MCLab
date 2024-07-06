#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(MCWIDGET_LIB)
#  define MCWIDGET_EXPORT Q_DECL_EXPORT
# else
#  define MCWIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define MCWIDGET_EXPORT
#endif

#define MCWIDGET_BEGIN_NAMESPACE namespace MCWidget {
#define MCWIDGET_END_NAMESPACE };

#define USE_MCWIDGET_NAMESPACE using namespace MCWidget;
