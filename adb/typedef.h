#pragma once

#include <QtCore/QtGlobal>

#ifndef AN_API
#if defined(AN_BUILD_ADB)
#define AN_API Q_DECL_EXPORT
#else
#define AN_API Q_DECL_IMPORT
#endif
#endif
