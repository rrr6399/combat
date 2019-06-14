// -*- c++ -*-

/*
 * Generic ORB definitions for Combat
 *
 * You *must* edit this file when trying to configure Combat to use
 * a "generic ORB" (--with-generic-orb). It is being read during the
 * configuration process and during the build.
 *
 * You do not need to touch this file when using one of the "supported"
 * ORBs - in that case, the ORB-specific `defines-ORB.h' file is used.
 */

/*
 * Add necessary #include <> statements here.
 *
 * The base file is usually called `CORBA.h' or similar. With some ORBs,
 * you may also need to include additional include files for the Interface
 * Repository, for the PortableServer module, or for the DynamicAny module.
 * If these files are in a non-standard location, you can use the configu-
 * ration option --with-includes to point to them.
 */

#include < * Edit Me! * >

/*
 * These probably need adjustment, too, the CORBA specs aren't very explicit
 * in what CORBA::Boolean should really be. Combat expects TRUE and FALSE
 * to be global symbols. The following should work and using the constants
 * 0 (FALSE) and 1 (TRUE) is actually recommended, but if you know a more
 * reasonable definition, you should use it. For example, TRUE and FALSE
 * might be enum values in the CORBA namespace, or `true' and `false' (from
 * the C++ bool type).
 */

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/*
 * Assume support of Namespaces and Exceptions.
 *
 * If your C++ compiler does not support namespaces, leave HAVE_NAMESPACE
 * undefined, and set
 *
 * #define COMBAT_NAMESPACE struct
 * #define COMBAT_EXPORT static
 * #define COMBAT_EXPORT_VAR static
 *
 * Note that this does not extend to whether the ORB uses a "namespace"
 * mapping or the alternative "flat" mapping itself: Combat always expects
 * to find CORBA::ORB_init(), and not CORBA_ORB_init().
 */

#ifndef HAVE_NAMESPACE
#define HAVE_NAMESPACE
#endif
#ifndef HAVE_EXCEPTIONS
#define HAVE_EXCEPTIONS
#endif
#define COMBAT_NAMESPACE namespace
#define COMBAT_EXPORT extern
#define COMBAT_EXPORT_VAR extern

