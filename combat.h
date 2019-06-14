// -*- c++ -*-

/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
#ifndef __COMBAT_H__
#define __COMBAT_H__

#ifndef TCL_THREADS
#define TCL_THREADS
#endif

/*
 * ORB-specific includes and defines
 */

#if defined(COMBAT_USE_MICO)
#include "defines-mico.h"
#elif defined(COMBAT_USE_ORBACUS)
#include "defines-orbacus.h"
#elif defined(COMBAT_USE_ORBIX)
#include "defines-orbix.h"
#else
#include "defines-generic.h"
#endif

#include <tcl.h>

/*
 * ----------------------------------------------------------------------
 *
 * All in the Combat Namespace
 *
 * ----------------------------------------------------------------------
 */

COMBAT_NAMESPACE Combat {

/*
 * ----------------------------------------------------------------------
 * Data types
 * ----------------------------------------------------------------------
 */

/*
 * Object information
 */

struct Context;
class PseudoObj;
class UniqueIdGenerator;
class InterfaceInfo;

struct Object {
  Object (Tcl_Interp *, Context *, const char *, CORBA::Object_ptr);
  Object (Tcl_Interp *, Context *, const char *, PseudoObj *);
  ~Object ();

  bool UpdateType ();
  bool UpdateType (const char *);
  bool UpdateType (CORBA::InterfaceDef_ptr);

  /*
   * Reference counting
   */

  int refs;

  /*
   * Generic info
   */

  Tcl_Interp * interp;
  Context * ctx;

  /*
   * This handle's command name
   */

  static UniqueIdGenerator IdFactory;
  char * name;

  /*
   * Info for "real" objects
   */

  CORBA::Object_var obj;
  InterfaceInfo * iface;

  /*
   * Info for pseudo objects
   */

  PseudoObj * pseudo;
};

/*
 * Base class for Requests
 */

class Request
{
public:
  Request ();
  virtual ~Request ();
  
  virtual int  Setup          (Tcl_Interp *, Context *,
			       int objc, Tcl_Obj *CONST []) = 0;
  virtual int  Invoke         (Tcl_Interp *) = 0;
  virtual int  GetResult      (Tcl_Interp *) = 0;
  virtual bool PollResult     (void) = 0;

  /*
   * Callback information. PerformCallback is called from the event
   * handler.
   */

  virtual void SetCallback    (Tcl_Interp *, Tcl_Obj *);
  virtual void PerformCallback();

  /*
   * Unique Id
   */

  virtual const char * get_id () const;

private:
  /*
   * Information for Callback
   */

  Tcl_Interp * cbinterp;
  Tcl_Obj * cbfunc;

  /*
   * Unique Id Generator
   */

  CORBA::String_var id;
  static UniqueIdGenerator IdFactory;
};

/*
 * ObjectRequest handles any invocations on Objects or Pseudo-Objects
 */

class ObjectRequest : virtual public Request
{
public:
  ObjectRequest (Object *);
  ~ObjectRequest ();

  int  Setup          (Tcl_Interp *, Context *,
		       int objc, Tcl_Obj *CONST []);
  int  SetupDii       (Tcl_Interp *, Context *, Tcl_Obj * CONST,
		       int objc, Tcl_Obj *CONST []);

  int  Invoke         (Tcl_Interp *);
  int  GetResult      (Tcl_Interp *);
  bool PollResult     (void);

private:
  int  SetupGet        (Tcl_Interp *, const char *,
			CORBA::AttributeDescription *);
  int  SetupSet        (Tcl_Interp *, const char *, Tcl_Obj *,
			CORBA::AttributeDescription *);
  int  SetupInvoke     (Tcl_Interp *, const char *,
			int, Tcl_Obj *CONST [],
			CORBA::OperationDescription *);
  bool SetupPseudo     (Tcl_Interp *, const char *, int,
			Tcl_Obj *CONST [], int *);
  bool SetupBuiltin    (Tcl_Interp *, const char *, int,
			Tcl_Obj *CONST [], int *);

  /*
   * Information for real ops
   */

  Object * obj;
  Context * ctx;
  bool is_finished;
  CORBA::Request_var req;
  CORBA::TypeCode_var rtype;
  Tcl_Obj * req_except;

  /*
   * Information for builtin ops
   */

  bool is_builtin;
  Tcl_Obj * builtin_result;

  /*
   * We need these for out/inout parameters
   */

  bool is_oneway;
  Tcl_Obj ** params;
  CORBA::ParDescriptionSeq * pds;
};

/*
 * Cache Interface Information
 */

class InterfaceInfo {
public:
  InterfaceInfo (CORBA::InterfaceDef_ptr,
		 const CORBA::InterfaceDef::FullInterfaceDescription &);
  ~InterfaceInfo ();

  const char * id ();
  bool lookup (const char *,
	       CORBA::OperationDescription *&,
	       CORBA::AttributeDescription *&);
  CORBA::InterfaceDef_ptr iface ();

private:
  typedef std::map<std::string, CORBA::OperationDescription *> OpMap;
  typedef std::map<std::string, CORBA::AttributeDescription *> AtMap;

  OpMap operations;
  AtMap attributes;
  CORBA::String_var repoid;
  CORBA::InterfaceDef_var ifd;
};

class InterfaceCache {
public:
  InterfaceCache ();
  ~InterfaceCache ();

  InterfaceInfo * insert (const char *);
  InterfaceInfo * insert (CORBA::InterfaceDef_ptr);
  void remove (const char *);

private:
  InterfaceInfo * insert (CORBA::InterfaceDef_ptr,
			  const CORBA::InterfaceDef::FullInterfaceDescription &);

  struct InterfaceRef {
    InterfaceInfo * desc;
    unsigned long refs;
  };
  typedef std::map<std::string, InterfaceRef> IfaceMap;
  IfaceMap interfaces;
};

/*
 * Base class for Pseudo Objects
 */

class PseudoObj
{
public:
  PseudoObj ();
  virtual ~PseudoObj ();

  Tcl_Obj * invoke (Tcl_Interp *, Context *, const char *,
		    int objc, Tcl_Obj *CONST []);

  virtual CORBA::Object_ptr get_managed (void);

  virtual Tcl_Obj * local_invoke (Tcl_Interp *, Context *, const char *,
				  int, Tcl_Obj *CONST []) = 0;

private:
  bool builtin_invoke (Tcl_Interp *, Context *, const char *,
		       int, Tcl_Obj *CONST [], Tcl_Obj **);
};

#if !defined(COMBAT_NO_SERVER_SIDE)

/*
 * POAManager pseudo object
 */

class POAManager : virtual public PseudoObj
{
public:
  POAManager (PortableServer::POAManager_ptr);
  ~POAManager ();

  CORBA::Object_ptr get_managed (void);

  Tcl_Obj * local_invoke (Tcl_Interp *, Context *, const char *,
			  int, Tcl_Obj *CONST []);

private:
  Tcl_Obj * activate         (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * hold_requests    (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * discard_requests (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * deactivate       (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);

  PortableServer::POAManager_ptr managed;
};

/*
 * ServerRequest pseudo object
 */

class ServerRequest : virtual public PseudoObj
{
public:
  ServerRequest (CORBA::ServerRequest_ptr);
  ~ServerRequest ();

  Tcl_Obj * local_invoke (Tcl_Interp *, Context *, const char *,
			  int, Tcl_Obj *CONST []);

private:
  Tcl_Obj * operation     (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * arguments     (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * set_result    (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * set_exception (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);

  CORBA::NVList_ptr args;
  CORBA::ServerRequest_ptr managed;
};

/*
 * POA pseudo object
 */

class POA : virtual public PseudoObj
{
public:
  POA (PortableServer::POA_ptr);
  ~POA ();

  CORBA::Object_ptr get_managed (void);

  Tcl_Obj * local_invoke (Tcl_Interp *, Context *, const char *,
			  int, Tcl_Obj *CONST []);

private:
  Tcl_Obj * create_POA              (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * find_POA                (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * destroy                 (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
				     
  Tcl_Obj * the_name                (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * the_parent              (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * the_POAManager          (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * the_activator           (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
				     
  Tcl_Obj * get_servant_manager     (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * set_servant_manager     (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * get_servant             (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * set_servant             (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
				     
  Tcl_Obj * activate_object         (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * activate_object_with_id (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * deactivate_object       (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * create_reference        (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * create_reference_with_id(Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
				     
  Tcl_Obj * servant_to_id           (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * servant_to_reference    (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * reference_to_servant    (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * reference_to_id         (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * id_to_servant           (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);
  Tcl_Obj * id_to_reference         (Tcl_Interp *, Context *,
				     int, Tcl_Obj *CONST []);

  PortableServer::POA_ptr managed;
};

/*
 * POACurrent
 */

class POACurrent : virtual public PseudoObj
{
public:
  POACurrent (PortableServer::Current_ptr);
  ~POACurrent ();

  CORBA::Object_ptr get_managed (void);

  Tcl_Obj * local_invoke (Tcl_Interp *, Context *, const char *,
			  int, Tcl_Obj *CONST []);

private:
  Tcl_Obj * get_POA (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);
  Tcl_Obj * get_object_id (Tcl_Interp *, Context *, int, Tcl_Obj *CONST []);

  PortableServer::Current_ptr managed;
};

/*
 * Tcl Servant, DSI servant and Servant Managers
 */

class Servant :
  virtual public PortableServer::ServantBase,
  virtual public PortableServer::RefCountServantBase
{
public:
  Servant (Tcl_Interp *, Tcl_Obj *, Context *);
  ~Servant ();

  virtual CORBA::Object_ptr _this () = 0;

protected:
  Tcl_Interp * interp;
  Tcl_Obj * obj;
  Context * ctx;
};

class DynamicServant :
  virtual public Servant,
  virtual public PortableServer::DynamicImplementation
{
public:
  DynamicServant (Tcl_Interp *, Tcl_Obj *, Context *,
		  CORBA::InterfaceDef_ptr);
  ~DynamicServant ();

  CORBA::Object_ptr _this ();

  void invoke (CORBA::ServerRequest_ptr);
  void invoke_delegate (CORBA::ServerRequest_ptr);
  CORBA::RepositoryId _primary_interface (const PortableServer::ObjectId &,
					  PortableServer::POA_ptr);

  CORBA::Boolean _is_a (const char *);

private:
  void dispatch_invoke   (const char *,
			  CORBA::ServerRequest_ptr,
			  CORBA::OperationDescription *);
  void dispatch_attr_get (const char *,
			  CORBA::ServerRequest_ptr,
			  CORBA::AttributeDescription *);
  void dispatch_attr_set (const char *,
			  CORBA::ServerRequest_ptr,
			  CORBA::AttributeDescription *);

  InterfaceInfo * iface;
};

class DynamicImplementation :
  virtual public Servant,
  virtual public PortableServer::DynamicImplementation
{
public:
  DynamicImplementation (Tcl_Interp *, Tcl_Obj *, Context *);
  ~DynamicImplementation ();

  CORBA::Object_ptr _this ();

  void invoke (CORBA::ServerRequest_ptr);
  CORBA::Boolean _is_a (const char *)
    throw (CORBA::SystemException);
  CORBA::RepositoryId _primary_interface (const PortableServer::ObjectId &,
					  PortableServer::POA_ptr);
};

class ServantActivator :
  virtual public Servant,
  virtual public POA_PortableServer::ServantActivator
{
public:
  ServantActivator (Tcl_Interp *, Tcl_Obj *, Context *);
  ~ServantActivator ();

  CORBA::Object_ptr _this ();

  PortableServer::Servant incarnate (const PortableServer::ObjectId &,
				     PortableServer::POA_ptr)
    throw (PortableServer::ForwardRequest, CORBA::SystemException);

  void etherealize (const PortableServer::ObjectId &,
		    PortableServer::POA_ptr,
		    PortableServer::Servant,
		    CORBA::Boolean, CORBA::Boolean)
    throw (CORBA::SystemException);
};

class ServantLocator :
  virtual public Servant,
  virtual public POA_PortableServer::ServantLocator
{
public:
  ServantLocator (Tcl_Interp *, Tcl_Obj *, Context *);
  ~ServantLocator ();

  CORBA::Object_ptr _this ();

  PortableServer::Servant preinvoke (const PortableServer::ObjectId &,
				     PortableServer::POA_ptr,
				     const char *,
				     PortableServer::ServantLocator::Cookie &)
    throw (PortableServer::ForwardRequest, CORBA::SystemException);

  void postinvoke (const PortableServer::ObjectId &,
		   PortableServer::POA_ptr,
		   const char *,
		   PortableServer::ServantLocator::Cookie,
		   PortableServer::Servant)
    throw (CORBA::SystemException);
};

class AdapterActivator :
  virtual public Servant,
  virtual public POA_PortableServer::AdapterActivator
{
public:
  AdapterActivator (Tcl_Interp *, Tcl_Obj *, Context *);
  ~AdapterActivator ();

  CORBA::Object_ptr _this ();

  CORBA::Boolean unknown_adapter (PortableServer::POA_ptr,
				  const char *)
    throw (CORBA::SystemException);
};

/*
 * if defined(COMBAT_NO_SERVER_SIDE)
 */

#endif

/*
 * Unique Id generator. Can an ULong as Id be too weak? Sure!
 */

class UniqueIdGenerator {
public:
  UniqueIdGenerator ();
  UniqueIdGenerator (const char *);
  ~UniqueIdGenerator ();

  char * new_id ();

private:
  int ulen, pfxlen;
  char * uid;
  char * prefix;
};

/*
 * Interpreter-specific context data:
 *   - Objects
 *   - Async Ops
 *   - Servants
 *
 * Global data:
 *   - The ORB
 *   - The Interface Repository
 *   - Table of Interps to Contexts
 */

struct Context {
  Context (void);
  ~Context ();

  /*
   * Tables for Active Objects, Async Ops and Servants
   */

  typedef std::map<std::string, Object *> ActiveObjTable;
  typedef std::map<std::string, Request *> RequestTable;

  ActiveObjTable active;
  RequestTable AsyncOps;
  RequestTable CbOps;

#if !defined(COMBAT_NO_SERVER_SIDE)
  typedef std::map<std::string, Servant *> ServantMap;
  ServantMap servants;
#endif

  /*
   * Do we need to call back an unhandled callback?
   */

  bool cbReady;
};

struct Global {
  Global (void);
  ~Global ();

  CORBA::ORB_ptr orb;
  CORBA::Repository_ptr repo;
  DynamicAny::DynAnyFactory_ptr daf;
  InterfaceCache icache;

#ifdef COMBAT_ORBACUS_LOCAL_REPO
  pid_t repopid;
#endif

  bool haveitcl;

  typedef std::map<Tcl_Interp *, Context *> CtxMap;
  CtxMap contexts;
};

/*
 * The global state
 */

COMBAT_EXPORT_VAR Global * GlobalData;

/*
 * ----------------------------------------------------------------------
 * Exported Functions
 * ----------------------------------------------------------------------
 */

// from typecode.cc

COMBAT_EXPORT_VAR Tcl_ObjType TypeCodeType;

COMBAT_EXPORT Tcl_Obj *           NewTypeCodeObj     (CORBA::TypeCode_ptr);
COMBAT_EXPORT CORBA::TypeCode_ptr GetTypeCodeFromObj (Tcl_Interp *,
						      Tcl_Obj *);

// from any.cc

COMBAT_EXPORT_VAR Tcl_ObjType AnyType;
COMBAT_EXPORT_VAR Tcl_ObjType OldListType;
COMBAT_EXPORT_VAR Tcl_ObjType * ListTypePtr;

COMBAT_EXPORT Tcl_Obj    * NewAnyObj     (Tcl_Interp *, Context *,
					  const CORBA::Any &);
COMBAT_EXPORT Tcl_Obj    * NewAnyObj     (const CORBA::Any &);
COMBAT_EXPORT CORBA::Any * GetAnyFromObj (Tcl_Interp *, Context *,
					  Tcl_Obj *,
					  const CORBA::TypeCode_ptr);

// from ir.cc

#if !defined(COMBAT_NO_COMBAT_IR)
COMBAT_EXPORT int       IR_Add      (Tcl_Interp *, Tcl_Obj *,
				     CORBA::Repository_ptr);
COMBAT_EXPORT int       IR_Add      (Tcl_Interp *, Tcl_Obj *,
				     CORBA::Repository_ptr,
				     const char *);
#endif

// from skel.cc

#if !defined(COMBAT_NO_SERVER_SIDE)
COMBAT_EXPORT const char * PortableServerCode;
#endif

// from COMBAT.cc

COMBAT_EXPORT_VAR Tcl_ObjType OldCmdType;
COMBAT_EXPORT_VAR Tcl_ObjType * CmdTypePtr;

COMBAT_EXPORT Tcl_Obj * InstantiateObj (Tcl_Interp *, Context *,
					CORBA::Object_ptr);
COMBAT_EXPORT Tcl_Obj * InstantiateObj (Tcl_Interp *, Context *,
					PseudoObj *);

COMBAT_EXPORT CORBA::Any * EncodeException (Tcl_Interp *, Context *, Tcl_Obj *);
COMBAT_EXPORT Tcl_Obj * DecodeException (Tcl_Interp *, Context *, const CORBA::Exception *);

#if !defined(COMBAT_NO_SERVER_SIDE)
COMBAT_EXPORT Tcl_Obj * ObjectId_to_Tcl_Obj (const PortableServer::ObjectId &);
COMBAT_EXPORT PortableServer::ObjectId * Tcl_Obj_to_ObjectId (Tcl_Obj *);
COMBAT_EXPORT Servant * FindServantByName (Tcl_Interp *, Context *, Tcl_Obj *);
#endif

// from event-*.cc

COMBAT_EXPORT int SetupORBEventHandler (Tcl_Interp *, Context *);

}; // namespace Combat

/*
 * extern "C" functions outside of a namespace to be on the safe side
 */

extern "C" {

// from combat.cc

int Combat_Invoke (ClientData, Tcl_Interp *, int, Tcl_Obj *CONST []);

// from any.cc

int Combat_ListFromAny (Tcl_Interp *, Tcl_Obj *);

Tcl_ThreadId getMainThread();

};

#endif
