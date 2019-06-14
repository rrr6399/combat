// -*- c++ -*-

/*
 * Hack for Mingw32 build.
 */

#if defined (__MINGW32__)
#ifndef _WINDOWS
#define _WINDOWS
#endif
#ifndef BUILD_MICO_DLL
#define BUILD_MICO_DLL
#endif
#endif

/*
 * MICO-specific definitions for Combat
 */

#include <CORBA.h>

#if MICO_BIN_VERSION < 0x020309
#define COMBAT_NAMESPACE MICO_NAMESPACE_DECL
#define COMBAT_EXPORT MICO_EXPORT_DECL
#define COMBAT_EXPORT_VAR MICO_EXPORT_VAR_DECL

#else

#ifndef HAVE_NAMESPACE
#define HAVE_NAMESPACE
#endif

#ifndef HAVE_EXCEPTIONS
#define HAVE_EXCEPTIONS
#endif

#define COMBAT_NAMESPACE namespace
#define COMBAT_EXPORT extern
#define COMBAT_EXPORT_VAR extern
#endif
