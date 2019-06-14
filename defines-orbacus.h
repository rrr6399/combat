// -*- c++ -*-

/*
 * ORBacus-specific definitions for Combat
 */

#include <OB/CORBA.h>
#include <OB/IFR.h>
#include <OB/RefCounted.h>

#ifdef COMBAT_ORBACUS_LOCAL_REPO
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

#define HAVE_NAMESPACE
#define HAVE_EXCEPTIONS
#define COMBAT_NAMESPACE namespace
#define COMBAT_EXPORT extern
#define COMBAT_EXPORT_VAR extern

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

