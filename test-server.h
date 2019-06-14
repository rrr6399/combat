/*
 * Generic server-side include file to mask differences between ORBs
 * We are supposed to pull in all necessary definitions to make a server.
 * For simplicity, we assume that the IDL file is named "test.idl"
 */

#ifndef __COMBAT_TEST_SERVER_H__
#define __COMBAT_TEST_SERVER_H__

#if defined(COMBAT_USE_MICO)

/*
 * MICO
 */

#include <test.h>

#elif defined(COMBAT_USE_ORBACUS)

/*
 * ORBacus
 */

#include <OB/CORBA.h>
#include <test_skel.h>

#ifndef TRUE
#define TRUE CORBA::TRUE
#define FALSE CORBA::FALSE
#endif

#elif defined(COMBAT_USE_ORBIX)

/*
 * ORBix
 */

#include <omg/orb.hh>
#include <omg/PortableServerS.hh>
#include <omg/DynamicAny.hh>
#include <test.h>
#include <testS.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#else

/*
 * Generic ORB
 */

#error "You must edit test-server.h to compile test suite or demos!"

#endif

/*
 * #ifndef __COMBAT_TEST_SERVER_H__
 */

#endif
