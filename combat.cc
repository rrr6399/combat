/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for MICO
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */

#include "combat.h"
#include "tclmap.h"
#include <stdlib.h>
#include <assert.h>
#include <string>

#ifdef COMBAT_USE_MICO
#include "mico-binder.h"
#endif

#if defined(COMBAT_USE_MICO) && MICO_BIN_VERSION > 0x020305
#include <mico/ir_creator.h>
#endif

char * combat_combat_id = "$Id: combat.cc,v 1.21 2004/07/21 02:39:33 fp Exp $";

/*
 * ----------------------------------------------------------------------
 * Features List
 * ----------------------------------------------------------------------
 */

struct features {
  const char * name;
  const char * version;
};

static struct features feature_list[] = {
  { "core",       "0.7" },
  { "async",      "0.7" },
  { "callback",   "0.7" },
  { "type",       "0.7" },
  { "dii",        "0.7" },
#if !defined(COMBAT_NO_SERVER_SIDE)
  { "poa",        "0.7" }, // ignored if [incr Tcl] is not available
#endif
#if defined(COMBAT_USE_MICO)
  { "mico::bind", "0.7" },
#endif
#if !defined(COMBAT_NO_COMBAT_IR)
  { "combat::ir", "0.7" },
#endif
  { NULL,       NULL }
};

/*
 * ----------------------------------------------------------------------
 * Data types implementation
 * ----------------------------------------------------------------------
 */

#ifdef HAVE_NAMESPACE
namespace Combat {
  Global * GlobalData = NULL;
  Tcl_ObjType OldCmdType;
  Tcl_ObjType * CmdTypePtr;
  Tcl_ObjType OldListType;
  Tcl_ObjType * ListTypePtr;
  UniqueIdGenerator Object::IdFactory("_combat_obj_");
};
#else
Combat::Global * Combat::GlobalData = NULL;
Tcl_ObjType Combat::OldCmdType;
Tcl_ObjType * Combat::CmdTypePtr;
Tcl_ObjType Combat::OldListType;
Tcl_ObjType * Combat::ListTypePtr;
Combat::UniqueIdGenerator Combat::Object::IdFactory ("_combat_obj_");
#endif

/*
 * Object information
 */

static char *
GetRepoIdFromObj (CORBA::Object_ptr obj)
{
#if defined(COMBAT_USE_MICO)
  if (!obj->_ior()) return NULL;
  return CORBA::string_dup (obj->_repoid());
#elif defined(COMBAT_USE_ORBACUS)
  OB::RefCountIOR_var ior;
  try {
    ior = obj->_OB_IOR();
  } catch (...) {
    return NULL;
  }
  if (!ior) return NULL;
  if (!ior->value.type_id.in()) return NULL;
  return CORBA::string_dup (ior->value.type_id.in());
#elif defined(COMBAT_USE_ORBIX)
  return obj->_it_get_type_id ();
#else
  return NULL;
#endif
}

Combat::Object::Object (Tcl_Interp * _interp, Context * _ctx,
			const char * cmdname, CORBA::Object_ptr _obj)
{
  refs   = 0;
  interp = _interp;
  ctx    = _ctx;
  iface  = NULL;
  obj    = _obj;
  pseudo = NULL;
  name   = CORBA::string_dup (cmdname);
  assert (!CORBA::is_nil (obj));

  /*
   * See if the object reference contains a Repository Id
   */

  char * repoid = GetRepoIdFromObj (obj);
  if (repoid) {
    UpdateType (repoid);
  }
  CORBA::string_free (repoid);
}

Combat::Object::Object (Tcl_Interp * _interp, Context * _ctx,
			const char * cmdname,
			PseudoObj * _pseudo)
{
  refs   = 0;
  interp = _interp;
  ctx    = _ctx;
  iface  = NULL;
  obj    = CORBA::Object::_nil ();
  pseudo = _pseudo;
  name   = CORBA::string_dup (cmdname);
  assert (pseudo);
}

/*
 * Update the object's type information using polled or available
 * information. Returns true if iface is actually updated.
 */

bool
Combat::Object::UpdateType ()
{
  CORBA::InterfaceDef_var ifd;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    ifd = obj->_get_interface ();
#ifdef HAVE_EXCEPTIONS
  } catch (...) {
    ifd = CORBA::InterfaceDef::_nil ();
  }
#endif

  return UpdateType (ifd);
}

bool
Combat::Object::UpdateType (const char * repoid)
{
  if (!repoid || !*repoid) {
    return false;
  }

  /*
   * Don't update to a base type
   */

  if (iface) {
    CORBA::InterfaceDef_var ifd = iface->iface ();
    if (ifd->is_a (repoid)) {
      return false;
    }
  }

  InterfaceInfo * ninfo = GlobalData->icache.insert (repoid);

  if (ninfo) {
    if (iface) {
      GlobalData->icache.remove (iface->id());
    }
    iface = ninfo;
    return true;
  }

  return false;
}

bool
Combat::Object::UpdateType (CORBA::InterfaceDef_ptr ifd)
{
  if (CORBA::is_nil (ifd)) {
    return false;
  }

  /*
   * Never "update" to a base type
   */

  if (iface) {
    CORBA::InterfaceDef_var oldifd = iface->iface ();
    CORBA::String_var nid = ifd->id ();
    if (oldifd->is_a (nid.in())) {
      return false;
    }
  }

  InterfaceInfo * ninfo = GlobalData->icache.insert (ifd);

  if (ninfo) {
    if (iface) {
      GlobalData->icache.remove (iface->id());
    }
    iface = ninfo;
    return true;
  }

  return false;
}

Combat::Object::~Object ()
{
  if (iface) {
    GlobalData->icache.remove (iface->id());
  }
  CORBA::string_free (name);
  delete pseudo;
}

/*
 * Interface Cache
 */

Combat::InterfaceInfo::InterfaceInfo (CORBA::InterfaceDef_ptr _ifd,
				      const CORBA::InterfaceDef::FullInterfaceDescription & id)
{
  CORBA::ULong i;
  for (i=0; i<id.operations.length(); i++) {
    CORBA::OperationDescription * od =
      new CORBA::OperationDescription (id.operations[i]);
    operations[id.operations[i].name.in()] = od;
  }
  for (i=0; i<id.attributes.length(); i++) {
    CORBA::AttributeDescription * ad =
      new CORBA::AttributeDescription (id.attributes[i]);
    attributes[id.attributes[i].name.in()] = ad;
  }
  repoid = CORBA::string_dup (id.id.in());
  ifd = CORBA::InterfaceDef::_duplicate (_ifd);
  assert (!CORBA::is_nil (ifd));
}

Combat::InterfaceInfo::~InterfaceInfo ()
{
  for (OpMap::iterator oi = operations.begin(); oi != operations.end(); oi++) {
    delete (*oi).second;
  }
  for (AtMap::iterator ai = attributes.begin(); ai != attributes.end(); ai++) {
    delete (*ai).second;
  }
}

const char *
Combat::InterfaceInfo::id ()
{
  return repoid.in();
}

CORBA::InterfaceDef_ptr
Combat::InterfaceInfo::iface ()
{
  return CORBA::InterfaceDef::_duplicate (ifd);
}

bool
Combat::InterfaceInfo::lookup (const char * name,
			       CORBA::OperationDescription *& od,
			       CORBA::AttributeDescription *& ad)
{
  OpMap::iterator oi = operations.find (name);
  if (oi != operations.end()) {
    od = (*oi).second;
    ad = NULL;
    return true;
  }
  AtMap::iterator ai = attributes.find (name);
  if (ai != attributes.end()) {
    od = NULL;
    ad = (*ai).second;
    return true;
  }
  return false;
}

Combat::InterfaceCache::InterfaceCache ()
{
}

Combat::InterfaceCache::~InterfaceCache ()
{
  IfaceMap::iterator ii;
  for (ii = interfaces.begin (); ii != interfaces.end(); ii++) {
    delete (*ii).second.desc;
  }
}

Combat::InterfaceInfo *
Combat::InterfaceCache::insert (const char * repoid)
{
  IfaceMap::iterator ii = interfaces.find (repoid);
  if (ii != interfaces.end()) {
    (*ii).second.refs++;
    return (*ii).second.desc;
  }
  if (!CORBA::is_nil (GlobalData->repo)) {
    InterfaceInfo * res;
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      CORBA::Contained_var cv = GlobalData->repo->lookup_id (repoid);
      CORBA::InterfaceDef_var ifd = CORBA::InterfaceDef::_narrow (cv);
      if (!CORBA::is_nil (ifd)) {
	CORBA::InterfaceDef::FullInterfaceDescription_var fid =
	  ifd->describe_interface ();
	res = insert (ifd.in(), fid.in());
      }
      else {
	res = NULL;
      }
#ifdef HAVE_EXCEPTIONS
    } catch (...) {
      res = NULL;
    }
#endif
    return res;
  }
  return NULL;
}

Combat::InterfaceInfo *
Combat::InterfaceCache::insert (CORBA::InterfaceDef_ptr ifd)
{
  InterfaceInfo * res;

  CORBA::String_var repoid = ifd->id ();
  if ((res = insert (repoid.in())) != NULL) {
    return res;
  }

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    CORBA::InterfaceDef::FullInterfaceDescription_var fid =
      ifd->describe_interface ();
    res = insert (ifd, fid.in());
#ifdef HAVE_EXCEPTIONS
  } catch (...) {
    res = NULL;
  }
#endif
  return NULL;
}

Combat::InterfaceInfo *
Combat::InterfaceCache::insert (CORBA::InterfaceDef_ptr ifd,
				const CORBA::InterfaceDef::FullInterfaceDescription & fid)
{
  InterfaceRef & ii = interfaces[fid.id.in()];
  ii.desc = new InterfaceInfo (ifd, fid);
  ii.refs = 1;
  return ii.desc;
}

void
Combat::InterfaceCache::remove (const char * repoid)
{
  IfaceMap::iterator ii = interfaces.find (repoid);
  assert (ii != interfaces.end());
  if (--(*ii).second.refs == 0) {
    delete (*ii).second.desc;
    interfaces.erase (ii);
  }
}

/*
 * Interpreter-specific Context
 */

Combat::Context::Context (void)
{
  cbReady = false;
}

Combat::Context::~Context ()
{
  /* Todo: clean up tables */
}

/*
 * Global State
 */

Combat::Global::Global (void)
{
  orb = CORBA::ORB::_nil ();
  repo = CORBA::Repository::_nil ();
  daf = DynamicAny::DynAnyFactory::_nil ();
#ifdef COMBAT_ORBACUS_LOCAL_REPO
  repopid = (pid_t) -1;
#endif
}

Combat::Global::~Global (void)
{
  /* Todo: clean up tables */
  if (!CORBA::is_nil (orb)) {
    orb->destroy ();
  }
  CORBA::release (repo);
  CORBA::release (daf);
  CORBA::release (orb);
#ifdef COMBAT_ORBACUS_LOCAL_REPO
  if (repopid != (pid_t) -1) {
    kill (repopid, SIGINT);
    waitpid (repopid, NULL, 0);
  }
#endif
}

/*
 * ----------------------------------------------------------------------
 * Helpers
 * ----------------------------------------------------------------------
 */

/*
 * Instantiate a CORBA object as a Tcl command.
 * The CORBA::Object is consumed.
 */

Tcl_Obj *
Combat::InstantiateObj (Tcl_Interp * interp, Context * ctx,
			CORBA::Object_ptr obj)
{
  CORBA::String_var name = Object::IdFactory.new_id ();
  Object * mobj = new Object (interp, ctx, name.in(), obj);
  ctx->active[name.in()] = mobj;
  Tcl_CreateObjCommand (interp, (char *) name.in(),
			Combat_Invoke, mobj, NULL);

  /*
   * Obtain a cmdName object. Tcl_ConvertToType calls our own
   * SetFromAny procedure, which plugs in the Combat::Object
   */

  Tcl_Obj * res = Tcl_NewStringObj ((char *) name.in(), -1);
  int inf = Tcl_ConvertToType (interp, res, CmdTypePtr);
  assert (inf == TCL_OK);
  return res;
}

/*
 * Instantiate a pseudo object as a Tcl command.
 */

Tcl_Obj *
Combat::InstantiateObj (Tcl_Interp * interp, Context * ctx,
			PseudoObj * pseudo)
{
  CORBA::String_var name = Object::IdFactory.new_id ();
  Object * mobj = new Object (interp, ctx, name.in(), pseudo);
  ctx->active[name.in()] = mobj;
  Tcl_CreateObjCommand (interp, (char *) name.in(),
			Combat_Invoke, mobj, NULL);

  /*
   * Obtain a cmdName object and plug in our object information
   */

  Tcl_Obj * res = Tcl_NewStringObj ((char *) name.in(), -1);
  int inf = Tcl_ConvertToType (interp, res, CmdTypePtr);
  assert (inf == TCL_OK);
  return res;
}

/*
 * Check if the Tcl procedure threw an exception or simply failed.
 */

CORBA::Any *
Combat::EncodeException (Tcl_Interp * interp,
			 Combat::Context * ctx,
			 Tcl_Obj * data)
{
  static const char * standard_exceptions[] = {
    "UNKNOWN", "BAD_PARAM", "NO_MEMORY", "IMP_LIMIT", "COMM_FAILURE",
    "INV_OBJREF", "NO_PERMISSION", "INTERNAL", "MARSHAL", "INITIALIZE",
    "NO_IMPLEMENT", "BAD_TYPECODE", "BAD_OPERATION", "NO_RESOURCES",
    "NO_RESPONSE", "PERSIST_STORE", "BAD_INV_ORDER", "TRANSIENT",
    "FREE_MEM", "INV_IDENT", "INV_FLAG", "INTF_REPOS", "BAD_CONTEXT",
    "OBJ_ADAPTER", "DATA_CONVERSION", "OBJECT_NOT_EXIST",
    "TRANSACTION_REQUIRED", "TRANSACTION_ROLLEDBACK",
    "INVALID_TRANSACTION", "INV_POLICY",
    NULL
  };

  CORBA::Any *res = NULL;
  Tcl_Obj *o1, *o2;
  const char *repoid;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
      (len != 1 && len != 2) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK) {
    return NULL;
  }

  repoid = Tcl_GetStringFromObj (o1, NULL);

  if (strncmp (repoid, "IDL:omg.org/CORBA/", 18) == 0) {
    CORBA::ULong index;
    for (index=0; standard_exceptions[index]; index++) {
      if (strncmp (repoid+18, standard_exceptions[index],
		   strlen (standard_exceptions[index])) == 0) {
	break;
      }
    }
    if (standard_exceptions[index]) {
      CORBA::CompletionStatus cs;
      CORBA::ULong minor;
      const char *smin, *str;
      Tcl_Obj *c1, *c2, *c3, *c4;
      char *sme;
      int clen;

      if (len != 2 || Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
	  Tcl_ListObjLength (NULL, o2, &clen) != TCL_OK || clen != 4 ||
	  Tcl_ListObjIndex (NULL, o2, 0, &c1) != TCL_OK ||
	  Tcl_ListObjIndex (NULL, o2, 1, &c2) != TCL_OK ||
	  Tcl_ListObjIndex (NULL, o2, 2, &c3) != TCL_OK ||
	  Tcl_ListObjIndex (NULL, o2, 3, &c4) != TCL_OK) {
	return NULL;
      }

      if (strcmp (Tcl_GetStringFromObj (c1, NULL), "minor") == 0 &&
	  strcmp (Tcl_GetStringFromObj (c3, NULL), "completion_status") == 0) {
	smin  = Tcl_GetStringFromObj (c2, NULL);
	str   = Tcl_GetStringFromObj (c4, NULL);
      }
      else if (strcmp (Tcl_GetStringFromObj (c1, NULL), "completion_status") == 0 &&
	       strcmp (Tcl_GetStringFromObj (c3, NULL), "minor") == 0) {
	str   = Tcl_GetStringFromObj (c2, NULL);
	smin  = Tcl_GetStringFromObj (c4, NULL);
      }
      else {
	return NULL;
      }

      minor = strtoul (smin, &sme, 0);

      if (*sme) {
	return NULL;
      }

      if (strncmp (str, "CORBA::", 7) == 0) {
	str += 7;
      }
      if (strcmp (str, "COMPLETED_YES") == 0) {
	cs = CORBA::COMPLETED_YES;
      }
      else if (strcmp (str, "COMPLETED_NO") == 0) {
	cs = CORBA::COMPLETED_NO;
      }
      else if (strcmp (str, "COMPLETED_MAYBE") == 0) {
	cs = CORBA::COMPLETED_MAYBE;
      }
      else {
	return NULL;
      }

      CORBA::Exception * ex = NULL;

#define COMBAT_MAKE_SYSEX(name) \
      if (strcmp (standard_exceptions[index], #name) == 0) \
        ex = new CORBA:: name (minor, cs);

      COMBAT_MAKE_SYSEX(UNKNOWN)
      COMBAT_MAKE_SYSEX(BAD_PARAM)
      COMBAT_MAKE_SYSEX(NO_MEMORY)
      COMBAT_MAKE_SYSEX(IMP_LIMIT)
      COMBAT_MAKE_SYSEX(COMM_FAILURE)
      COMBAT_MAKE_SYSEX(INV_OBJREF)
      COMBAT_MAKE_SYSEX(NO_PERMISSION)
      COMBAT_MAKE_SYSEX(INTERNAL)
      COMBAT_MAKE_SYSEX(MARSHAL)
      COMBAT_MAKE_SYSEX(INITIALIZE)
      COMBAT_MAKE_SYSEX(NO_IMPLEMENT)
      COMBAT_MAKE_SYSEX(BAD_TYPECODE)
      COMBAT_MAKE_SYSEX(BAD_OPERATION)
      COMBAT_MAKE_SYSEX(NO_RESOURCES)
      COMBAT_MAKE_SYSEX(NO_RESPONSE)
      COMBAT_MAKE_SYSEX(PERSIST_STORE)
      COMBAT_MAKE_SYSEX(BAD_INV_ORDER)
      COMBAT_MAKE_SYSEX(TRANSIENT)
      COMBAT_MAKE_SYSEX(FREE_MEM)
      COMBAT_MAKE_SYSEX(INV_IDENT)
      COMBAT_MAKE_SYSEX(INV_FLAG)
      COMBAT_MAKE_SYSEX(INTF_REPOS)
      COMBAT_MAKE_SYSEX(BAD_CONTEXT)
      COMBAT_MAKE_SYSEX(OBJ_ADAPTER)
      COMBAT_MAKE_SYSEX(DATA_CONVERSION)
      COMBAT_MAKE_SYSEX(OBJECT_NOT_EXIST)
      COMBAT_MAKE_SYSEX(TRANSACTION_REQUIRED)
      COMBAT_MAKE_SYSEX(TRANSACTION_ROLLEDBACK)
      COMBAT_MAKE_SYSEX(INVALID_TRANSACTION)
      COMBAT_MAKE_SYSEX(INV_POLICY)

#undef COMBAT_MAKE_SYSEX

      assert (ex);
      res = new CORBA::Any;
      *res <<= ex;
    }
  }

  if (!res && !CORBA::is_nil (GlobalData->repo)) {
    CORBA::ExceptionDef_var exd;

#ifdef HAVE_EXCEPTIONS
    try {
#endif
      CORBA::Contained_var contained =
	GlobalData->repo->lookup_id (repoid);
      exd = CORBA::ExceptionDef::_narrow (contained);
#ifdef HAVE_EXCEPTIONS
    } catch (...) {
      exd = CORBA::ExceptionDef::_nil();
    }
#endif
      
    if (CORBA::is_nil (exd)) {
      return NULL;
    }
    
    CORBA::TypeCode_var etc = exd->type ();
    res = Combat::GetAnyFromObj (interp, ctx, data, etc);
  }

  return res;
}

Tcl_Obj *
Combat::DecodeException (Tcl_Interp * interp, Context * ctx,
			 const CORBA::Exception * ex)
{
  assert (ex);

  const CORBA::SystemException * sysex =
    CORBA::SystemException::_downcast (ex);

  if (sysex) {
    Tcl_Obj *o[2], *c[4];
    char tmp[256];

    sprintf (tmp, "%lu", (unsigned long) sysex->minor());
    c[0] = Tcl_NewStringObj ("minor", 5);
    c[1] = Tcl_NewStringObj (tmp, -1);
    c[2] = Tcl_NewStringObj ("completed", 9);

    switch (sysex->completed()) {
    case CORBA::COMPLETED_YES:
      c[3] = Tcl_NewStringObj ("COMPLETED_YES", 13);
      break;

    case CORBA::COMPLETED_NO:
      c[3] = Tcl_NewStringObj ("COMPLETED_NO", 12);
      break;

    case CORBA::COMPLETED_MAYBE:
      c[3] = Tcl_NewStringObj ("COMPLETED_MAYBE", 15);
      break;

    default:
      assert (0);
    }

    const char * repoid = NULL;

#define COMBAT_MAKE_SYSEX(name) \
    if (CORBA:: name ::_downcast (sysex)) \
      repoid = "IDL:omg.org/CORBA/" #name ":1.0";

      COMBAT_MAKE_SYSEX(UNKNOWN)
      COMBAT_MAKE_SYSEX(BAD_PARAM)
      COMBAT_MAKE_SYSEX(NO_MEMORY)
      COMBAT_MAKE_SYSEX(IMP_LIMIT)
      COMBAT_MAKE_SYSEX(COMM_FAILURE)
      COMBAT_MAKE_SYSEX(INV_OBJREF)
      COMBAT_MAKE_SYSEX(NO_PERMISSION)
      COMBAT_MAKE_SYSEX(INTERNAL)
      COMBAT_MAKE_SYSEX(MARSHAL)
      COMBAT_MAKE_SYSEX(INITIALIZE)
      COMBAT_MAKE_SYSEX(NO_IMPLEMENT)
      COMBAT_MAKE_SYSEX(BAD_TYPECODE)
      COMBAT_MAKE_SYSEX(BAD_OPERATION)
      COMBAT_MAKE_SYSEX(NO_RESOURCES)
      COMBAT_MAKE_SYSEX(NO_RESPONSE)
      COMBAT_MAKE_SYSEX(PERSIST_STORE)
      COMBAT_MAKE_SYSEX(BAD_INV_ORDER)
      COMBAT_MAKE_SYSEX(TRANSIENT)
      COMBAT_MAKE_SYSEX(FREE_MEM)
      COMBAT_MAKE_SYSEX(INV_IDENT)
      COMBAT_MAKE_SYSEX(INV_FLAG)
      COMBAT_MAKE_SYSEX(INTF_REPOS)
      COMBAT_MAKE_SYSEX(BAD_CONTEXT)
      COMBAT_MAKE_SYSEX(OBJ_ADAPTER)
      COMBAT_MAKE_SYSEX(DATA_CONVERSION)
      COMBAT_MAKE_SYSEX(OBJECT_NOT_EXIST)
      COMBAT_MAKE_SYSEX(TRANSACTION_REQUIRED)
      COMBAT_MAKE_SYSEX(TRANSACTION_ROLLEDBACK)
      COMBAT_MAKE_SYSEX(INVALID_TRANSACTION)
      COMBAT_MAKE_SYSEX(INV_POLICY)

#undef COMBAT_MAKE_SYSEX

    assert (repoid);

    o[0] = Tcl_NewStringObj ((char *) repoid, -1);
    o[1] = Tcl_NewListObj (4, c);
    return Tcl_NewListObj (2, o);
  }

  /*
   * Try some known user exceptions
   */

  if (CORBA::ORB::InvalidName::_downcast (ex) != NULL) {
    Tcl_Obj *o[2];
    o[0] = Tcl_NewStringObj ("IDL:omg.org/CORBA/ORB/InvalidName:1.0", 37);
    o[1] = Tcl_NewObj ();
    return Tcl_NewListObj (2, o);
  }

  /*
   * Must be a user exception.
   */

  CORBA::Any any;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    any <<= *ex;
#ifdef HAVE_EXCEPTIONS
  } catch (...) {
    Tcl_Obj *o[2];
    o[0] = Tcl_NewStringObj ("Unknown User Exception", 22);
    o[1] = Tcl_NewObj ();
    return Tcl_NewListObj (2, o);
  }
#endif

  return Combat::NewAnyObj (interp, ctx, any);
}

#if !defined(COMBAT_NO_SERVER_SIDE)

Tcl_Obj *
Combat::ObjectId_to_Tcl_Obj (const PortableServer::ObjectId & id)
{
  return Tcl_NewStringObj ((char *) id.get_buffer(), id.length());
}

PortableServer::ObjectId *
Combat::Tcl_Obj_to_ObjectId (Tcl_Obj * data)
{
  int len;
  const char * str = Tcl_GetStringFromObj (data, &len);
  return new PortableServer::ObjectId (len, len, (CORBA::Octet *) str);
}

Combat::Servant *
Combat::FindServantByName (Tcl_Interp * interp, Combat::Context * ctx,
			   Tcl_Obj * obj)
{
  Tcl_CmdInfo info;
  int nidx;

  const char * cmdname = Tcl_GetStringFromObj (obj, NULL);

  if (!Tcl_GetCommandInfo (interp, (char *) cmdname, &info)) {
    Tcl_AppendResult (interp, "error: oops: could not find servant: \"",
		      cmdname, "\"", NULL);
    return NULL;
  }

  /*
   * To get the full command name, use the fully qualified namespace name
   * and the tail of the command name
   */

  for (nidx = strlen (cmdname); nidx > 0; nidx--) {
    if (cmdname[nidx-1] == ':') {
      break;
    }
  }

  char * fqn = CORBA::string_alloc (strlen (info.namespacePtr->fullName) +
				    strlen (cmdname+nidx) + 3);
  sprintf (fqn, "%s%s%s", info.namespacePtr->fullName,
	   (info.namespacePtr->fullName[0] != ':' ||
	    info.namespacePtr->fullName[1] != ':' ||
	    info.namespacePtr->fullName[2] != '\0') ? "::" : "",
	   cmdname + nidx);

  Combat::Context::ServantMap::iterator it;
  it = ctx->servants.find (fqn);

  if (it == ctx->servants.end()) {
    Tcl_AppendResult (interp, "error: oops: servant not initialized: \"",
		      fqn, "\"", NULL);
    return NULL;
  }

  CORBA::string_free (fqn);

  return (*it).second;
}

/*
 * if defined(COMBAT_NO_SERVER_SIDE)
 */

#endif

/*
 * Unique Id generator
 */

Combat::UniqueIdGenerator::UniqueIdGenerator ()
{
  uid = NULL;
  prefix = NULL;
  pfxlen = 0;
}

Combat::UniqueIdGenerator::UniqueIdGenerator (const char * pfx)
{
  uid = NULL;
  prefix = CORBA::string_dup (pfx);
  pfxlen = strlen (prefix);
}

Combat::UniqueIdGenerator::~UniqueIdGenerator ()
{
  CORBA::string_free (uid);
  CORBA::string_free (prefix);
}

char *
Combat::UniqueIdGenerator::new_id ()
{
  char * id;

  /*
   * Generate a new unique id
   */

  if (uid == NULL) {
    ulen = 1;
    uid  = CORBA::string_alloc (ulen);
    assert (uid);
    uid[0] = '0';
    uid[1] = 0;
  }
  else {
    int i;
    for (i=0; i<ulen; i++) {
      if (uid[i] != '9')
	break;
      uid[i] = '0';
    }
    if (i == ulen) {
      CORBA::string_free (uid);
      uid = CORBA::string_alloc (++ulen);
      assert (uid);
      for (i=0; i<ulen-1; i++) {
	uid[i] = '0';
      }
      uid[ulen-1] = '1';
      uid[ulen] = 0;
    }
    else {
      uid[i] = uid[i] + 1;
    }
  }
  id = CORBA::string_alloc (ulen + pfxlen);
  assert (id);
  if (prefix) strcpy (id, prefix);
  strcpy (id+pfxlen, uid);

  return id;
}

/*
 * ----------------------------------------------------------------------
 * Tcl Interface
 * ----------------------------------------------------------------------
 */

extern "C" {

/*
 * This event handler is responsible for invoking callbacks for non-DII
 * requests, i.e. requests that are not handled by the ORB-specific event
 * handler, such as invocations on pseudo objects, or internal operations.
 * Such requests must finish synchronously; their readiness is signalled
 * via Context::cbReady.
 */

struct Combat_Event {
  struct Tcl_Event ev;
  Combat::Context * ctx;
};

static int
Combat_HandleEvent (Tcl_Event * evPtr, int flags)
{
  assert (flags & TCL_FILE_EVENTS);

  Combat_Event * ev = (Combat_Event *) evPtr;
  Combat::Context * ctx = ev->ctx;

  /*
   * PerformCallback is supposed to call corba::request get, which
   * alters our vector<>, so the iterator becomes invalid after
   * executing a callback
   */

  Combat::Context::RequestTable::iterator el = ctx->CbOps.begin ();
  while (el != ctx->CbOps.end()) {
    if ((*el).second->PollResult()) {
      (*el).second->PerformCallback();
      el = ctx->CbOps.begin();
    }
    else {
      el++;
    }
  }

  return 1;
}

static void
Combat_SetupEvents (ClientData clientData, int flags)
{
  if (!(flags & TCL_FILE_EVENTS)) {
    return;
  }

  if (((Combat::Context *) clientData)->cbReady) {
    Tcl_Time tm;
    tm.sec  = 0;
    tm.usec = 0;
    Tcl_SetMaxBlockTime (&tm);
  }
}

static void
Combat_CheckEvents (ClientData clientData, int flags)
{
  if (!(flags & TCL_FILE_EVENTS)) {
    return;
  }

  Combat::Context * ctx = (Combat::Context *) clientData;

  if (ctx->cbReady) {
    Combat_Event * ev = (Combat_Event *) Tcl_Alloc (sizeof (Combat_Event));
    ev->ev.proc = Combat_HandleEvent;
    ev->ctx     = ctx;
    Tcl_QueueEvent ((struct Tcl_Event *) ev, TCL_QUEUE_TAIL);
    ctx->cbReady = false;
  }
}

/*
 * Handler when interp is deleted
 */

void
Combat_DeleteLocal (ClientData, Tcl_Interp * interp)
{
  Combat::Global::CtxMap::iterator it = 
    Combat::GlobalData->contexts.find (interp);
  assert (it != Combat::GlobalData->contexts.end());
  delete (*it).second;
  Combat::GlobalData->contexts.erase (it);
}

/*
 * Handler for program exit
 */

void
Combat_DeleteGlobal (ClientData)
{
  if (Combat::GlobalData != NULL) {
    delete Combat::GlobalData;
    Combat::GlobalData = NULL;
  }
}

/*
 * Handle object invocations; this function is installed as the
 * command behind each object
 */

int
Combat_Invoke (ClientData clientData, Tcl_Interp *interp,
	       int objc, Tcl_Obj *CONST objv[])
{
  Combat::Object * obj = (Combat::Object *) clientData;
  const char * objname = Tcl_GetStringFromObj (objv[0], NULL);
  Combat::Request * req;
  int async=0, option=1;
  Tcl_Obj * callback=NULL;

  if (objc == 1) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      objname, " op ?arguments?\"", NULL);
    return TCL_ERROR;
  }

  const char * opname = Tcl_GetStringFromObj (objv[option], NULL);

  while (*opname == '-' && option < objc) {
    if (strcmp (opname, "-async") == 0) {
      async = 1;
    }
    else if (strcmp (opname, "-callback") == 0) {
      if (option+1 >= objc) {
	Tcl_AppendResult (interp, "error: -callback needs a parameter", NULL);
	return TCL_ERROR;
      }
      option++;
      async = 1;
      callback = objv[option];
    }
    else if (strcmp (opname, "--") == 0) {
      option++;
      opname = Tcl_GetStringFromObj (objv[option], NULL);
      break;
    }
    else {
      Tcl_AppendResult (interp, "error: unknown option \"", opname,
			"\"", NULL);
      return TCL_ERROR;
    }
    option++;
    opname = Tcl_GetStringFromObj (objv[option], NULL);
  }

  /*
   * Setup invocation
   */

  req = new Combat::ObjectRequest (obj);

  if (req->Setup (interp, obj->ctx, objc-option, objv+option) != TCL_OK) {
    delete req;
    return TCL_ERROR;
  }

  if (callback) {
    req->SetCallback (interp, callback);
  }

  /*
   * Invoke
   */

  if (req->Invoke (interp) != TCL_OK) {
    delete req;
    return TCL_ERROR;
  }

  /*
   * Synchronous operation
   */

  if (!async) {
    int res;
    res = req->GetResult (interp);
    delete req;
    return res;
  }

  /*
   * Asynchronous operation: return a handle
   */

  if (callback) {
    obj->ctx->CbOps[req->get_id()] = req;
    if (req->PollResult()) {
      obj->ctx->cbReady = true;
    }
  }
  else {
    obj->ctx->AsyncOps[req->get_id()] = req;
  }

  Tcl_SetResult (interp, (char *) req->get_id(), TCL_VOLATILE);
  return TCL_OK;
}

/*
 * Initialize and connect to an ORB
 * corba::init ?orb-args ...? ?other-args ...?
 */

static int
Combat_Init_Cmd (ClientData clientData, Tcl_Interp *interp,
		 int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  char **myargv, **cpargv;
  int i, res, myargc=objc;
  CORBA::Object_var oir;

  if (!CORBA::is_nil (Combat::GlobalData->orb)) {
    return TCL_OK;
  }

  /*
   * Process parameters
   */

  if ((myargv = (char **) malloc ((myargc+1) * sizeof (char *))) == NULL) {
    Tcl_SetResult (interp, "oops: out of memory", TCL_STATIC);
    return TCL_ERROR;
  }
  if ((cpargv = (char **) malloc ((myargc+1) * sizeof (char *))) == NULL) {
    Tcl_SetResult (interp, "oops: out of memory", TCL_STATIC);
    free (myargv);
    return TCL_ERROR;
  }

  for (i=0; i<=myargc; i++) {
    myargv[i] = cpargv[i] = NULL;
  }

  for (i=0; i<myargc; i++) {
    const char * strarg = Tcl_GetStringFromObj (objv[i], NULL);
    if ((cpargv[i] = myargv[i] = strdup (strarg)) == NULL) {
      Tcl_SetResult (interp, "oops: out of memory", TCL_STATIC);
      res = TCL_ERROR;
      goto cleanup;
    }
  }

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    Combat::GlobalData->orb = CORBA::ORB_init (myargc, myargv);
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    Combat::GlobalData->orb = CORBA::ORB::_nil ();
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
  }
#endif

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    res = TCL_ERROR;
    goto cleanup;
  }

  /*
   * Register Event Handlers
   */

  if (Combat::SetupORBEventHandler (interp, ctx) != TCL_OK) {
    res = TCL_ERROR;
    goto cleanup;
  }

  Tcl_CreateEventSource (Combat_SetupEvents,
			 Combat_CheckEvents,
			 (ClientData) ctx);

  /*
   * Get IR. Ignore if not available.
   */

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    oir = Combat::GlobalData->orb->resolve_initial_references ("InterfaceRepository");
    Combat::GlobalData->repo = CORBA::Repository::_narrow (oir);
#ifdef HAVE_EXCEPTIONS
  }
  catch (CORBA::Exception &ex) {
    Combat::GlobalData->repo = CORBA::Repository::_nil ();
  }
#endif

  /*
   * Get DynAnyFactory
   */

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    oir = Combat::GlobalData->orb->resolve_initial_references ("DynAnyFactory");
    Combat::GlobalData->daf = DynamicAny::DynAnyFactory::_narrow (oir);
#ifdef HAVE_EXCEPTIONS
  }
  catch (CORBA::Exception &ex) {
    Combat::GlobalData->daf = DynamicAny::DynAnyFactory::_nil ();
  }
#endif

  if (CORBA::is_nil (Combat::GlobalData->daf)) {
    Tcl_SetResult (interp, "oops: could not resolve DynAnyFactory",
		   TCL_STATIC);
    CORBA::release (Combat::GlobalData->repo);
    CORBA::release (Combat::GlobalData->orb);
    Combat::GlobalData->repo = CORBA::Repository::_nil ();
    Combat::GlobalData->orb = CORBA::ORB::_nil ();
    res = TCL_ERROR;
    goto cleanup;
  }

  /*
   * push back args that were not consumed by the orb
   */

  for (i=1; i<myargc; i++) {
    Tcl_AppendElement (interp, myargv[i]);
  }

  res = TCL_OK;

cleanup:
  for (i=0; i<myargc; i++) {
    free (cpargv[i]);
  }

  free (cpargv);
  free (myargv);

  return res;
}

/*
 * corba::feature names
 * corba::feature require ?-exact? <feature> ?version?
 */

static bool
Combat_VersionCompare (const char * v1, const char * v2, bool exact)
{
  unsigned long num_1, num_2;
  char *e1, *e2;

  if (v1 && *v1) {
    num_1 = strtoul (v1, &e1, 0);
    if (v1==e1 || !e1 || (*e1 != '.' && *e1 != '\0')) return false;
    if (*(v1 = e1) == '.') v1++;
  }
  else {
    num_1 = 0;
  }

  if (v2 && *v2) {
    num_2 = strtoul (v2, &e2, 0);
    if (v2==e2 || !e2 || (*e2 != '.' && *e2 != '\0')) return false;
    if (*(v2 = e2) == '.') v2++;
  }
  else {
    num_2 = 0;
  }

  if (num_1 != num_2) {
    return false;
  }

  if (num_1 == 0) {
    exact = true;
  }

  while (v1 && *v1 && v2 && *v2) {
    num_1 = strtoul (v1, &e1, 0);
    if (v1==e1 || !e1 || (*e1 != '.' && *e1 != '\0')) return false;
    if (*(v1 = e1) == '.') v1++;

    num_2 = strtoul (v2, &e2, 0);
    if (v2==e2 || !e2 || (*e2 != '.' && *e2 != '\0')) return false;
    if (*(v2 = e2) == '.') v2++;

    if (num_1 > num_2 && !exact) {
      return true;
    }
    else if (num_1 < num_2) {
      return false;
    }
    else if (num_1 != num_2 && exact) {
      return false;
    }
  }

  if ((!v1 || !*v1) && v2 && *v2) {
    return false;
  }
  else if (v1 && *v1 && (!v2 || !*v2) && exact) {
    return false;
  }

  return true;
}

static int
Combat_Feature (ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
  const char * what;

  if (objc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " names\" or \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " require ?-exact? feature ?version?\"",
		      NULL);
    return TCL_ERROR;
  }

  what = Tcl_GetStringFromObj (objv[1], NULL);

  if (strcmp (what, "names") == 0) {
    if (objc != 2) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" names\"", NULL);
      return TCL_ERROR;
    }

    Tcl_Obj * res = Tcl_NewObj ();

    for (struct features *fl=feature_list; fl->name; fl++) {
      if (strcmp (fl->name, "poa") == 0 && !Combat::GlobalData->haveitcl) {
	continue;
      }
      Tcl_Obj * m = Tcl_NewStringObj ((char *) fl->name, -1);
      Tcl_ListObjAppendElement (NULL, res, m);
    }

    Tcl_SetObjResult (interp, res);
  }
  else if (strcmp (what, "require") == 0) {
    const char *feature=NULL, *version=NULL;
    bool exact=false;
    bool options=true;
    int par;

    for (par=2; par<objc; par++) {
      const char * opt = Tcl_GetStringFromObj (objv[par], NULL);

      if (strcmp (opt, "-exact") == 0 && options) {
	exact = true;
      }
      else if (strcmp (opt, "--") == 0 && options) {
	options = false;
      }
      else if (!feature) {
	feature = opt;
      }
      else if (!version) {
	version = opt;
      }
      else {
	break;
      }
    }

    if (par<objc || !feature) {
      Tcl_AppendResult (interp, "usage: \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" require ?-exact? feature ?version?\"",
			NULL);
      return TCL_ERROR;
    }

    for (struct features *fl=feature_list; fl->name; fl++) {
      if (strcmp (fl->name, "poa") == 0 && !Combat::GlobalData->haveitcl) {
	continue;
      }
      if (strcmp (fl->name, feature) == 0) {
	if (!version || Combat_VersionCompare (fl->version, version, exact)) {
	  Tcl_SetResult (interp, (char *) fl->version, TCL_STATIC);
	  return TCL_OK;
	}
      }
    }
    
    Tcl_AppendResult (interp, "error: feature \"", feature,
		      "\" not supported", NULL);
    if (version) {
      Tcl_AppendResult (interp, " in version \"", version, "\"", NULL);
    }

    return TCL_ERROR;
  }
  else {
    Tcl_AppendResult (interp, "usage: \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " names\" or \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " require ?-exact? feature ?version?\"",
		      NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

/*
 * corba::string_to_object IOR
 */

static int
Combat_String_To_Object (ClientData clientData, Tcl_Interp *interp,
			 int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  Tcl_Obj * res;

  if (objc != 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " IOR\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  const char * ior = Tcl_GetStringFromObj (objv[1], NULL);
  CORBA::Object_ptr obj;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    obj = Combat::GlobalData->orb->string_to_object (ior);
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    obj = CORBA::Object::_nil ();
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
  }
#endif

  if (CORBA::is_nil (obj)) {
    return TCL_ERROR;
  }

  if ((res = Combat::InstantiateObj (interp, ctx, obj)) == NULL) {
    CORBA::release (obj);
    return TCL_ERROR;
  }

  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * corba::object_to_string handle
 */

static int
Combat_Object_To_String (ClientData clientData, Tcl_Interp *interp,
			 int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

  if (objc != 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " handle\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  const char * cmdname = Tcl_GetStringFromObj (objv[1], NULL);
  Tcl_CmdInfo info;

  if (!Tcl_GetCommandInfo (interp, (char *) cmdname, &info)) {
    Tcl_AppendResult (interp, "error: no such object: ", cmdname, NULL);
    return TCL_ERROR;
  }

  Combat::Object * obj = (Combat::Object *) info.objClientData;

  if (CORBA::is_nil (obj->obj)) {
    Tcl_AppendResult (interp, "error: not an object: ", cmdname, NULL);
    return TCL_ERROR;
  }

  CORBA::String_var ior = Combat::GlobalData->orb->object_to_string (obj->obj);
  res = Tcl_NewStringObj ((char *) ior.in(), -1);

  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * corba::list_initial_services
 */

static int
Combat_List_Initial_Services (ClientData clientData, Tcl_Interp *interp,
			      int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

  if (objc != 1) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  CORBA::ORB::ObjectIdList_var olv = Combat::GlobalData->orb->list_initial_services ();

  res = Tcl_NewObj ();
  for (CORBA::ULong i=0; i<olv->length(); i++) {
    Tcl_Obj * elm = Tcl_NewStringObj ((char *) olv[i].in(), -1);
    Tcl_ListObjAppendElement (interp, res, elm);
  }

  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * corba::resolve_initial_references id
 */

static int
Combat_Resolve_Initial_References (ClientData clientData, Tcl_Interp *interp,
				   int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  Tcl_Obj * res;

  if (objc != 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " id\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  const char * ident = Tcl_GetStringFromObj (objv[1], NULL);
  CORBA::Object_ptr obj;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    obj = Combat::GlobalData->orb->resolve_initial_references (ident);
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    obj = CORBA::Object::_nil ();
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
  }
#endif

  if (CORBA::is_nil (obj)) {
    return TCL_ERROR;
  }

#if !defined(COMBAT_NO_SERVER_SIDE)

  /*
   * Special handling for RootPOA and POACurrent pseudo objects
   */

  if (strcmp (ident, "RootPOA") == 0) {
    PortableServer::POA_var poa = PortableServer::POA::_narrow (obj);
    Combat::POA * mpoa = new Combat::POA (poa);
    res = Combat::InstantiateObj (interp, ctx, mpoa);
    CORBA::release (obj);
  }
  else if (strcmp (ident, "POACurrent") == 0) {
    PortableServer::Current_var cur = PortableServer::Current::_narrow (obj);
    Combat::POACurrent * mcur = new Combat::POACurrent (cur);
    res = Combat::InstantiateObj (interp, ctx, mcur);
    CORBA::release (obj);
  }
  else {
#endif
    if ((res = Combat::InstantiateObj (interp, ctx, obj)) == NULL) {
      CORBA::release (obj);
      return TCL_ERROR;
    }
#if !defined(COMBAT_NO_SERVER_SIDE)
  }
#endif

  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * corba::register_initial_reference id
 */

static int
Combat_Register_Initial_Reference (ClientData clientData, Tcl_Interp *interp,
				   int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;

  if (objc != 3) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " id obj\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  const char * ident = Tcl_GetStringFromObj (objv[1], NULL);
  const char * cmdname = Tcl_GetStringFromObj (objv[2], NULL);
  Tcl_CmdInfo info;

  if (!Tcl_GetCommandInfo (interp, (char *) cmdname, &info)) {
    Tcl_AppendResult (interp, "error: no such object: ", cmdname, NULL);
    return TCL_ERROR;
  }

  Combat::Object * obj = (Combat::Object *) info.objClientData;

  if (CORBA::is_nil (obj->obj)) {
    Tcl_AppendResult (interp, "error: not an object: ", cmdname, NULL);
    return TCL_ERROR;
  }

#if defined(COMBAT_USE_MICO)
  Combat::GlobalData->orb->set_initial_reference (ident, obj->obj);
#elif defined(COMBAT_USE_ORBACUS)
  Combat::GlobalData->orb->register_initial_reference (ident, obj->obj);
#else
  Tcl_AppendResult (interp, "error: register_initial_reference not supported",
		    NULL);
  return TCL_ERROR;
#endif
  return TCL_OK;
}

/*
 * corba::dii handle ?-async? operation spec ?args?
 */

static int
Combat_Dii (ClientData clientData, Tcl_Interp *interp,
	    int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  Combat::ObjectRequest * req;
  int async=0, option=1;
  Tcl_Obj * callback=NULL;

  if (objc < 3) {
    const char * cmdname = Tcl_GetStringFromObj (objv[0], NULL);
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      cmdname, " ?options? handle spec ?arguments?\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  const char * objname = Tcl_GetStringFromObj (objv[option], NULL);

  while (*objname == '-' && option < objc) {
    if (strcmp (objname, "-async") == 0) {
      async = 1;
    }
    else if (strcmp (objname, "-callback") == 0) {
      if (option+1 >= objc) {
	Tcl_AppendResult (interp, "error: -callback needs a parameter", NULL);
	return TCL_ERROR;
      }
      option++;
      async = 1;
      callback = objv[option];
    }
    else if (strcmp (objname, "--") == 0) {
      option++;
      objname = Tcl_GetStringFromObj (objv[option], NULL);
      break;
    }
    else {
      Tcl_AppendResult (interp, "error: unknown option \"", objname,
			"\"", NULL);
      return TCL_ERROR;
    }
    option++;
    objname = Tcl_GetStringFromObj (objv[option], NULL);
  }

  Tcl_CmdInfo info;

  if (!Tcl_GetCommandInfo (interp, (char *) objname, &info)) {
    Tcl_AppendResult (interp, "error: no such object: ", objname, NULL);
    return TCL_ERROR;
  }

  Combat::Object * obj = (Combat::Object *) info.objClientData;

  if (CORBA::is_nil (obj->obj)) {
    Tcl_AppendResult (interp, "error: not an object: ", objname, NULL);
    return TCL_ERROR;
  }

  /*
   * Get Spec
   */

  if (++option >= objc) {
    const char * cmdname = Tcl_GetStringFromObj (objv[0], NULL);
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      cmdname, " ?options? handle spec ?arguments?\"", NULL);
    return TCL_ERROR;
  }

  Tcl_Obj *CONST spec = objv[option++];

  /*
   * Setup invocation
   */

  req = new Combat::ObjectRequest (obj);

  if (req->SetupDii (interp, obj->ctx, spec,
		     objc-option, objv+option) != TCL_OK) {
    delete req;
    return TCL_ERROR;
  }

  if (callback) {
    req->SetCallback (interp, callback);
  }

  /*
   * Invoke
   */

  if (req->Invoke (interp) != TCL_OK) {
    delete req;
    return TCL_ERROR;
  }

  /*
   * Synchronous operation
   */

  if (!async) {
    int res;
    res = req->GetResult (interp);
    delete req;
    return res;
  }

  /*
   * Asynchronous operation: return a handle
   */

  if (callback) {
    obj->ctx->CbOps[req->get_id()] = req;
    if (req->PollResult()) {
      obj->ctx->cbReady = true;
    }
  }
  else {
    obj->ctx->AsyncOps[req->get_id()] = req;
  }

  Tcl_SetResult (interp, (char *) req->get_id(), TCL_VOLATILE);
  return TCL_OK;
}

/*
 * corba::const repoid-or-scoped-name
 */

static int
Combat_Const (ClientData clientData, Tcl_Interp *interp,
	      int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;

  if (objc != 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " repoid-or-scoped-name\"", NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  if (CORBA::is_nil (Combat::GlobalData->repo)) {
    Tcl_AppendResult (interp, "oops: no local repository available", NULL);
    return TCL_ERROR;
  }

  const char * repoid = Tcl_GetStringFromObj (objv[1], NULL);

  CORBA::Contained_var obj = Combat::GlobalData->repo->lookup_id (repoid);

  if (CORBA::is_nil (obj)) {
    obj = Combat::GlobalData->repo->lookup (repoid);
  }

  if (CORBA::is_nil (obj)) {
    Tcl_AppendResult (interp, "error: could not find \"", repoid,
		      "\" in repository", NULL);
    return TCL_ERROR;
  }

  CORBA::ConstantDef_var cd = CORBA::ConstantDef::_narrow (obj);

  if (CORBA::is_nil (cd)) {
    Tcl_AppendResult (interp, "error: \"", repoid, "\" is not a constant",
		      NULL);
    return TCL_ERROR;
  }

  CORBA::Any * val = cd->value();
  CORBA::Any any;
  any <<= val;

  Tcl_Obj * res = Combat::NewAnyObj (interp, ctx, any);
  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * Request handler, for asynchronous operations
 *
 * corba::request poll ?ops ...?
 * corba::request wait ?ops ...?
 * corba::request get op
 */

static int
Combat_Request (ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  const char *what;

  if (objc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " poll ?ops?\" or \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " wait ?ops?\" or \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " get op\"",
		      NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  what = Tcl_GetStringFromObj (objv[1], NULL);

  /*
   * Get result
   */

  if (strcmp (what, "get") == 0) {
    if (objc != 3) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" get <op>\"", NULL);
      return TCL_ERROR;
    }

    const char * op = Tcl_GetStringFromObj (objv[2], NULL);
    Combat::Context::RequestTable::iterator el;

    if ((el = ctx->AsyncOps.find (op)) != ctx->AsyncOps.end()) {
      Combat::Request * req = (*el).second;
      ctx->AsyncOps.erase (el);
      int res = req->GetResult (interp);
      delete req;
      return res;
    }

    if ((el = ctx->CbOps.find (op)) != ctx->CbOps.end()) {
      Combat::Request * req = (*el).second;
      ctx->CbOps.erase (el);
      int res = req->GetResult (interp);
      delete req;
      return res;
    }
    
    Tcl_AppendResult (interp, "error: not an active operation handle: \"",
		      op, "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Wait/Poll for results
   */

  while (42) {
    if (objc == 2) {
      /*
       * Check all active ops
       */
      Combat::Context::RequestTable::iterator el;
      for (el = ctx->AsyncOps.begin(); el != ctx->AsyncOps.end(); el++) {
	if ((*el).second->PollResult ()) {
	  Tcl_SetResult (interp, (char *) (*el).first.c_str(), TCL_VOLATILE);
	  return TCL_OK;
	}
      }
      if (ctx->AsyncOps.size() == 0) {
	// no active async operations
	return TCL_OK;
      }
    }
    else {
      /*
       * Check arguments
       */
      for (int i=2; i<objc; i++) {
	const char * name = Tcl_GetStringFromObj (objv[i], NULL);
	Combat::Context::RequestTable::iterator el;
	if ((el = ctx->AsyncOps.find (name)) == ctx->AsyncOps.end()) {
	  Tcl_AppendResult (interp,
			    "error: not an active operation handle: \"",
			    name, "\"", NULL);
	  return TCL_ERROR;
	}
	Combat::Request * req = (*el).second;
	if (req->PollResult ()) {
	  Tcl_SetResult (interp, (char *) name, TCL_VOLATILE);
	  return TCL_OK;
	}
      }
    }

    if (strcmp (what, "poll") == 0) {
      break;
    }

    /*
     * The above loop is pretty costly, therefore we don't just process
     * one event before trying again, but all events that are on line
     */

    Tcl_DoOneEvent (0);
    while (Tcl_DoOneEvent (TCL_DONT_WAIT));
  }

  return TCL_OK;
}

/*
 * Manipulate the Interface Repository
 *
 * combat::ir add ir-description-seq
 * combat::ir add repoid-or-scoped-name ir-description
 */

#if !defined(COMBAT_NO_COMBAT_IR)

static int
Combat_IR (ClientData clientData, Tcl_Interp *interp,
	   int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  const char * what;
  Tcl_Obj * res;

  if (objc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " option arg ?...?\"",
		      NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  what = Tcl_GetStringFromObj (objv[1], NULL);

  if (strcmp (what, "set") == 0) {
    if (objc != 3) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			"set <handle>\"", NULL);
      return TCL_ERROR;
    }
    CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[2],
					      CORBA::_tc_Object);
    if (!any) {
      Tcl_AppendResult (interp, "error: \"", objv[2],
			"\": not a handle", NULL);
      return TCL_ERROR;
    }
    CORBA::Object_var obj;
    *any >>= CORBA::Any::to_object (obj);
    CORBA::Repository_ptr repo = CORBA::Repository::_narrow (obj);
    if (CORBA::is_nil (repo)) {
      Tcl_AppendResult (interp, "error: \"", objv[2],
			"\": not an Interface Repository", NULL);
      return TCL_ERROR;
    }
    CORBA::release (Combat::GlobalData->repo);
    Combat::GlobalData->repo = repo;
    return TCL_OK;
  }

#ifdef COMBAT_USE_ORBACUS
#ifdef COMBAT_ORBACUS_LOCAL_REPO
  /*
   * Start and connect to a private Interface Repository
   */

  if (CORBA::is_nil (Combat::GlobalData->repo)) {
    int fds[2];
    pipe (fds);

    switch (Combat::GlobalData->repopid = fork()) {
    case -1:
      Tcl_SetResult (interp, "oops: failed to fork irserv", TCL_STATIC);
      return TCL_ERROR;

    case 0: /* Child starts irserv */
      close (fds[0]);
      dup2  (fds[1], 1);
      close (fds[1]);
      execlp ("irserv", "irserv", "--ior", NULL);
      exit  (1);

    default: /* Parent continues to be Tcl */
      {
	string ifrior;
	char c;

	/*
	 * Read IFR's IOR from its stdout
	 */

	close (fds[1]);
	do {
	  if (read (fds[0], &c, 1) == 0)
	    break;
	  if (c == '\r' || c == '\n')
	    break;
	  ifrior += c;
	}
	while (42);
	close (fds[0]);

	try {
	  CORBA::Object_var obj =
	    Combat::GlobalData->orb->string_to_object (ifrior.c_str());
	  bool ifrisok = false;
	  
	  /*
	   * IFR may still need some time and answers early invocations with
	   * CORBA::TRANSIENT. Keep sending invocations until it responds
	   * with a value.
	   */

	  for (CORBA::ULong p=0; p<10 && !ifrisok; sleep(1), p++) {
	    CORBA::Repository_var repo = CORBA::Repository::_nil ();

	    try {
	      repo = CORBA::Repository::_narrow (obj);
	      CORBA::Contained_var cv = repo->lookup_id ("IDL:FooBar:1.0");
	      ifrisok = true;
	    }
	    catch (CORBA::TRANSIENT &ex) {
	    }
	  }

	  if (ifrisok) {
	    Combat::GlobalData->repo = CORBA::Repository::_narrow (obj);
	  }
	} catch (...) {
	}

	if (CORBA::is_nil (Combat::GlobalData->repo)) {
	  Tcl_SetResult (interp, "oops: failed to fork irserv", TCL_STATIC);
	  kill (Combat::GlobalData->repopid, SIGINT);
	  waitpid (Combat::GlobalData->repopid, NULL, 0);
	  Combat::GlobalData->repopid = (pid_t) -1;
	  return TCL_ERROR;
	}
      }
    }
  }
#endif
#endif

#if defined(COMBAT_USE_MICO) && MICO_BIN_VERSION > 0x020305
  if (CORBA::is_nil (Combat::GlobalData->repo)) {
    /*
     * Start a private Interface Repository
     */

    Combat::GlobalData->repo =
      MICO::create_interface_repository (Combat::GlobalData->orb);
  }
#endif

  if (CORBA::is_nil (Combat::GlobalData->repo)) {
    Tcl_AppendResult (interp, "oops: no local repository available", NULL);
    return TCL_ERROR;
  }

  if (strcmp (what, "add") == 0) {
    if (objc != 3 && objc != 4) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" add <ir-description-seq>\" or \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" add repoid-or-scoped-name <ir-description>\"",
			NULL);
      return TCL_ERROR;
    }

    if (objc == 3) {
#ifdef HAVE_EXCEPTIONS
      try {
#endif
	if (Combat::IR_Add (interp, objv[2],
			     Combat::GlobalData->repo) != TCL_OK) {
	  return TCL_ERROR;
	}
#ifdef HAVE_EXCEPTIONS
      } catch (CORBA::Exception &ex) {
	Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
	return TCL_ERROR;
      }
#endif
    }
    else {
      const char * repoid = Tcl_GetStringFromObj (objv[2], NULL);
      if (Combat::IR_Add (interp, objv[3], Combat::GlobalData->repo,
			   repoid) != TCL_OK) {
	return TCL_ERROR;
      }
    }
    res = Tcl_NewObj ();
  }
  else {
    Tcl_AppendResult (interp, "illegal option \"",
		      Tcl_GetStringFromObj (objv[1], NULL),
		      "\": should be add",
		      NULL);
    return TCL_ERROR;
  }

  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * if defined(COMBAT_NO_COMBAT_IR)
 */

#endif

/*
 * TypeCode handling
 *
 * corba::type of repoid-or-scoped-name
 * corba::type match TypeCode value
 * corba::type equivalent TypeCode TypeCode
 */

static int
Combat_Type (ClientData clientData, Tcl_Interp *interp,
	     int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  const char * what;
  Tcl_Obj * res;

  if (objc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " of, match or equivalent\"",
		      NULL);
    return TCL_ERROR;
  }

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  what = Tcl_GetStringFromObj (objv[1], NULL);

  if (strcmp (what, "of") == 0) {
    if (objc != 3) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" of repoid-or-scoped-name\"", NULL);
      return TCL_ERROR;
    }

    if (CORBA::is_nil (Combat::GlobalData->repo)) {
      Tcl_AppendResult (interp, "oops: no local repository available", NULL);
      return TCL_ERROR;
    }

    const char * repoid = Tcl_GetStringFromObj (objv[2], NULL);
    CORBA::Contained_var obj = Combat::GlobalData->repo->lookup_id (repoid);

    if (CORBA::is_nil (obj)) {
      obj = Combat::GlobalData->repo->lookup (repoid);
    }

    CORBA::IDLType_var idl = CORBA::IDLType::_narrow (obj);
    CORBA::TypeCode_var ttc;

    if (CORBA::is_nil (idl)) {
      CORBA::ExceptionDef_var ed;
      if (!CORBA::is_nil (ed = CORBA::ExceptionDef::_narrow (obj))) {
	ttc = ed->type ();
      }
      else {
	Tcl_AppendResult (interp, "error: could not find type \"",
			  repoid, "\" in repository", NULL);
	return TCL_ERROR;
      }
    }
    else {
      ttc = idl->type();
    }

    res = Combat::NewTypeCodeObj (ttc.in());
  }
  else if (strcmp (what, "match") == 0) {
    if (objc != 4) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" match typecode value\"", NULL);
      return TCL_ERROR;
    }

    CORBA::TypeCode_var tc = Combat::GetTypeCodeFromObj (interp, objv[2]);

    if (CORBA::is_nil (tc)) {
      return TCL_ERROR;
    }

    CORBA::Any_var any = Combat::GetAnyFromObj (interp, ctx, objv[3], tc);

    if (!any) {
      res = Tcl_NewIntObj (0);
    }
    else {
      res = Tcl_NewIntObj (1);
    }
  }
  else if (strcmp (what, "equivalent") == 0) {
    if (objc != 4) {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			" equivalent typecode typecode\"", NULL);
      return TCL_ERROR;
    }

    CORBA::TypeCode_var tc1 = Combat::GetTypeCodeFromObj (interp, objv[2]);
    
    if (CORBA::is_nil (tc1)) {
      return TCL_ERROR;
    }

    CORBA::TypeCode_var tc2 = Combat::GetTypeCodeFromObj (interp, objv[3]);
    
    if (CORBA::is_nil (tc2)) {
      return TCL_ERROR;
    }

    if (tc1->equivalent (tc2)) {
      res = Tcl_NewIntObj (1);
    }
    else {
      res = Tcl_NewIntObj (0);
    }
  }
  else {
    Tcl_AppendResult (interp, "error: illegal option: \"", what,
		      "\": should be of, match or equivalent", NULL);
    return TCL_ERROR;
  }

  Tcl_SetObjResult (interp, res);
  return TCL_OK;
}

/*
 * Mico Binder
 *
 * mico::bind ?-addr addr? repoid
 */

#ifdef COMBAT_USE_MICO
static int
Combat_Bind (ClientData clientData, Tcl_Interp *interp,
	     int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;
  Combat::Request * req;
  int async=0, option=1;
  Tcl_Obj * callback=NULL;

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  const char * opname = Tcl_GetStringFromObj (objv[1], NULL);

  while (*opname == '-' && option < objc) {
    if (strcmp (opname, "-async") == 0) {
      async = 1;
    }
    else if (strcmp (opname, "-callback") == 0) {
      if (option+1 >= objc) {
	Tcl_AppendResult (interp, "error: -callback needs a parameter", NULL);
	return TCL_ERROR;
      }
      option++;
      async = 1;
      callback = objv[option];
    }
    else if (strcmp (opname, "--") == 0) {
      option++;
      opname = Tcl_GetStringFromObj (objv[option], NULL);
      break;
    }
    else {
      break;
    }
    option++;
    opname = Tcl_GetStringFromObj (objv[option], NULL);
  }

  /*
   * Setup bind
   */

  req = new MicoBinder::BindRequest ();

  if (req->Setup (interp, ctx, objc-option, objv+option) != TCL_OK) {
    delete req;
    return TCL_ERROR;
  }

  if (callback) {
    req->SetCallback (interp, callback);
  }

  /*
   * Invoke
   */

  if (req->Invoke (interp) != TCL_OK) {
    delete req;
    return TCL_ERROR;
  }

  /*
   * Synchronous operation
   */

  if (!async) {
    int res;
    res = req->GetResult (interp);
    delete req;
    return res;
  }

  /*
   * Asynchronous operation: return a handle
   */

  if (callback) {
    ctx->CbOps[req->get_id()] = req;
    if (req->PollResult()) {
      ctx->cbReady = true;
    }
  }
  else {
    ctx->AsyncOps[req->get_id()] = req;
  }

  Tcl_SetResult (interp, (char *) req->get_id(), TCL_VOLATILE);
  return TCL_OK;
}
#endif

/*
 * Internal function for handling of Combat::Servant's
 *
 * combat::servant NewServant obj repoid
 * combat::servant DeleteServant obj
 * combat::servant _default_POA obj
 * combat::servant _this obj
 */

#if !defined(COMBAT_NO_SERVER_SIDE)

static int
Combat_Servant (ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
  Combat::Context * ctx = (Combat::Context *) clientData;

  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    if (Combat_Init_Cmd (clientData, interp, 0, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  if (objc < 3) {
    Tcl_AppendResult (interp, "error: illegal call to mico::servant",
		      NULL);
    return TCL_ERROR;
  }

  const char * what = Tcl_GetStringFromObj (objv[1], NULL);
  const char * name = Tcl_GetStringFromObj (objv[2], NULL);

  if (strcmp (what, "NewServant") == 0) {
    if (objc != 4) {
      Tcl_AppendResult (interp, "error: illegal call to mico::servant",
			NULL);
      return TCL_ERROR;
    }

    const char * repoid = Tcl_GetStringFromObj (objv[3], NULL);
    Combat::Servant * serv;
    
    if (strcmp (repoid, "IDL:omg.org/PortableServer/ServantActivator:1.0") == 0) {
      serv = new Combat::ServantActivator (interp, objv[2], ctx);
    }
    else if (strcmp (repoid, "IDL:omg.org/PortableServer/ServantLocator:1.0") == 0) {
      serv = new Combat::ServantLocator (interp, objv[2], ctx);
    }
    else if (strcmp (repoid, "IDL:omg.org/PortableServer/AdapterActivator:1.0") == 0) {
      serv = new Combat::AdapterActivator (interp, objv[2], ctx);
    }
    else if (strcmp (repoid, "IDL:omg.org/PortableServer/DynamicImplementation:1.0") == 0) {
      serv = new Combat::DynamicImplementation (interp, objv[2], ctx);
    }
    else {
      if (CORBA::is_nil (Combat::GlobalData->repo)) {
	Tcl_AppendResult (interp, "oops: no local repository available", NULL);
	return TCL_ERROR;
      }
      CORBA::Contained_var cv =	Combat::GlobalData->repo->lookup_id (repoid);
      CORBA::InterfaceDef_var iface = CORBA::InterfaceDef::_narrow (cv);
      if (CORBA::is_nil (iface)) {
	Tcl_AppendResult (interp, "error: could not find \"", repoid,
			  "\" in local repository", NULL);
	return TCL_ERROR;
      }
      serv = new Combat::DynamicServant (interp, objv[2], ctx, iface);
    }
    ctx->servants[name] = serv;
  }
  else if (strcmp (what, "DeleteServant") == 0) {
    Combat::Context::ServantMap::iterator it =
      ctx->servants.find (name);
    if (it != ctx->servants.end()) {
      Combat::Servant * serv = (*it).second;
      ctx->servants.erase (it);
      serv->_remove_ref();
    }
  }
  else if (strcmp (what, "_default_POA") == 0) {
    /*
     * This is called from PortableServer::ServantBase::_default_POA
     * Calling _default_POA on the object would call Combat::Servant's
     * _default_POA, invoke Tcl and lead to infinite recursion. So
     * explicitely call Servant::_default_POA().
     */
    Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[2]);
    PortableServer::POA_var poa =
      serv->PortableServer::ServantBase::_default_POA ();
    Combat::POA * mpoa = new Combat::POA (poa);
    Tcl_Obj * res = Combat::InstantiateObj (interp, ctx, mpoa);
    Tcl_SetObjResult (interp, res);
  }
  else if (strcmp (what, "_this") == 0) {
    Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[2]);
    CORBA::Object_ptr obj;
    assert (serv);
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      obj = serv->_this ();
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
      return TCL_ERROR;
    }
#endif
    Tcl_Obj * res = Combat::InstantiateObj (interp, ctx, obj);
    Tcl_SetObjResult (interp, res);
  }
  else {
    Tcl_AppendResult (interp, "error: illegal op for mico::servant: \"",
		      what, "\"", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

/*
 * if defined(COMBAT_NO_SERVER_SIDE)
 */

#endif

/*
 * Throw an Exception
 *
 * corba::throw <ex-data>
 *
 * This function doesn't really do that much, but it looks good in code ...
 */

static int
Combat_Throw (ClientData clientData, Tcl_Interp *interp,
	      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " <ex-data>\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Return a list of the IDL type and the exception contents,
   * and then throw an error.
   */

  Tcl_SetObjResult (interp, objv[1]);
  return TCL_ERROR;
}

/*
 * Throw an Exception
 *
 * corba::try proc ?catch {repo-id ?var?} proc ...? ?finally proc?
 *
 * This function doesn't really do that much, but it looks good in code ...
 */

static int
Combat_Try (ClientData clientData, Tcl_Interp *interp,
	    int objc, Tcl_Obj *CONST objv[])
{
  /*
   * first check for syntactic correctness before doing anything
   */

  if (objc < 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " <proc> ?catch {repo-id ?var?} proc ...?",
		      " ?finally proc?\"", NULL);
    return TCL_ERROR;
  }

  char * str;
  int i=2, l, finally=0;
  bool has_catch = false;

  while (i<objc) {
    str = Tcl_GetStringFromObj (objv[i], NULL);

    if (strcmp (str, "catch") == 0) {
      if (i+2 >= objc ||
	  Tcl_ListObjLength (NULL, objv[i+1], &l) != TCL_OK ||
	  (l != 1 && l != 2)) {
	Tcl_AppendResult (interp, "wrong # args: should be \"",
			  Tcl_GetStringFromObj (objv[0], NULL),
			  " <proc> ?catch {repo-id ?var?} proc ...?",
			  " ?finally proc?\"", NULL);
	return TCL_ERROR;
      }
      has_catch = true;
      i += 3;
    }
    else if (strcmp (str, "finally") == 0) {
      finally = i;
      if (i+2 != objc) {
	Tcl_AppendResult (interp, "wrong # args: should be \"",
			  Tcl_GetStringFromObj (objv[0], NULL),
			  " <proc> ?catch {repo-id ?var?} proc ...?",
			  " ?finally proc?\"", NULL);
	return TCL_ERROR;
      }
      i += 2;
      break;
    }
  }

  /*
   * Eval main procedure
   */

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  int res = Tcl_EvalObj (interp, objv[1]);
#else
  int res = Tcl_EvalObjEx (interp, objv[1], 0);
#endif

  /*
   * In case of error, check the catch clauses
   */

  if (res == TCL_ERROR) {
    Tcl_Obj * ex = Tcl_GetObjResult (interp);
    Tcl_Obj * repoobj;
    char * repoid;

    if (Tcl_ListObjLength (NULL, ex, &l) != TCL_OK ||
	(l != 1 && l != 2) ||
	Tcl_ListObjIndex (NULL, ex, 0, &repoobj) != TCL_OK) {
      repoid = NULL; /* does not look like a CORBA exception */
    }
    else {
      repoid = Tcl_GetStringFromObj (repoobj, NULL);
    }

    i = 2;

    Tcl_Obj * catchexvar;

    while (i<objc) {
      str = Tcl_GetStringFromObj (objv[i], NULL);

      if (strcmp (str, "catch") == 0) {
	Tcl_Obj * catchidobj;
	char * catchid;
	Tcl_ListObjLength (NULL, objv[i+1], &l);
	Tcl_ListObjIndex (NULL, objv[i+1], 0, &catchidobj);
	catchid = Tcl_GetStringFromObj (catchidobj, NULL);

	if ((repoid != NULL && strcmp (repoid, catchid) == 0) ||
	    strcmp (catchid, "...") == 0) {
	  if (l == 2) {
	    Tcl_ListObjIndex (NULL, objv[i+1], 1, &catchexvar);
	  }
	  else {
	    catchexvar = NULL;
	  }
	  break;
	}
	i += 3;
      }
      else if (strcmp (str, "finally") == 0) {
	i = objc;
	break;
      }
    }

    /*
     * Found?
     */

    if (i < objc) {
      /*
       * Then set variable and call the proc
       */
      if (catchexvar) {
	Tcl_ObjSetVar2 (interp, catchexvar, NULL, ex, 0);
      }
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
      res = Tcl_EvalObj (interp, objv[i+2]);
#else
      res = Tcl_EvalObjEx (interp, objv[i+2], 0);
#endif
    }

    /*
     * If there is no catch clause, swallow errors
     */

    if (!has_catch) {
      res = TCL_OK;
    }
  }

  /*
   * Execute "finally" clause, preserving the interp
   */

  if (finally) {
    Tcl_Obj * oldres = Tcl_GetObjResult (interp);
    Tcl_IncrRefCount (oldres);
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
    int finally_res = Tcl_EvalObj (interp, objv[finally+1]);
#else
    int finally_res = Tcl_EvalObjEx (interp, objv[finally+1], 0);
#endif

    /*
     * An Error in the finally clause takes precedence
     */

    if (finally_res != TCL_OK) {
      Tcl_DecrRefCount (oldres);
      return finally_res;
    }

    /*
     * Else restore the old result
     */

    Tcl_SetObjResult (interp, oldres);
  }

  /*
   * Pass along return code
   */

  return res;
}

static int
Combat_Shutdown (ClientData clientData, Tcl_Interp *interp,
		 int objc, Tcl_Obj *CONST objv[])
{
  if (CORBA::is_nil (Combat::GlobalData->orb)) {
    return TCL_OK;
  }

  if (objc != 2) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " wait_for_completion\"", NULL);
    return TCL_ERROR;
  }

  int bv;
  if ((Tcl_GetBooleanFromObj (interp, objv[1], &bv)) != TCL_OK) {
    Tcl_AppendResult (interp, "error: was expecting boolean value ",
		      "but got \"", Tcl_GetStringFromObj (objv[1], NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  Combat::GlobalData->orb->shutdown (bv ? TRUE : FALSE);
  return TCL_OK;
}

/*
 * corba::duplicate ?typecode? value
 *
 * Noop for compatibility with Combat/Tcl
 */

static int
Combat_Duplicate (ClientData clientData, Tcl_Interp *interp,
		  int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2 && objc != 3) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " ?typecode? value\"", NULL);
    return TCL_ERROR;
  }

  Tcl_SetObjResult (interp, objv[objc-1]);
  return TCL_OK;
}

/*
 * corba::release ?typecode? value
 *
 * Noop for compatibility with Combat/Tcl
 */

static int
Combat_Release (ClientData clientData, Tcl_Interp *interp,
		int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2 && objc != 3) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      " ?typecode? value\"", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 * Handling for Tcl's hijacked cmdName type.
 * ----------------------------------------------------------------------
 */

/*
 * SetFromAny -- this should never be invoked for a Handle, because
 * they are already created as cmdName objects. So we ought to be
 * able to safely invoke the original setFromAnyProc.
 *
 * However, this is not true for upvar'd and global'd variables pointing
 * to a handle. So we must check if the string representation of obj
 * happens to be a handle.
 *
 * We must bootstrap our context from the interp's `ctx' variable to
 * gain access to our table of active objects.
 */

static int
Combat_Handle_SetFromAny (Tcl_Interp * interp, Tcl_Obj * obj)
{
  Combat::Object * tobj = NULL;
  Combat::Context * ctx = NULL;
  int result;

  /*
   * For some reason, this function is called during cleanup by
   * Wish in Win32, so protect against that.
   */

  if (Combat::GlobalData != NULL) {
    Combat::Global::CtxMap::iterator cit =
      Combat::GlobalData->contexts.find (interp);
    if (cit != Combat::GlobalData->contexts.end()) {
      ctx = (*cit).second;

      /*
       * Check if this command is a handle
       */
  
      const char * objName = Tcl_GetStringFromObj (obj, NULL);
      Combat::Context::ActiveObjTable::iterator ait =
	ctx->active.find (objName);
    
      if (ait != ctx->active.end()) {
	tobj = (*ait).second;
	tobj->refs++;
      }
    }
  }

  result = Combat::OldCmdType.setFromAnyProc (interp, obj);

  if (result == TCL_OK && tobj != NULL) {
    obj->internalRep.twoPtrValue.ptr2 = (VOID *) (void *) tobj;
  }

  return result;
}

/*
 * According to Tcl, this is never called. The original method
 * just panics.
 */

static void
Combat_Handle_UpdateString (Tcl_Obj * obj)
{
  Combat::OldCmdType.updateStringProc (obj);
}

/*
 * Hm, can this happen? An object is duplicated if it is about to be
 * modified, and we cannot allow modifications on a handle. Let's
 * assert(0) here.
 */

static void
Combat_Handle_DupInternal (Tcl_Obj * src, Tcl_Obj * dup)
{
  if (src->internalRep.twoPtrValue.ptr2) {
    assert (0);
  }
  Combat::OldCmdType.dupIntRepProc (src, dup);
}

/*
 * Delete the command associated with this handle
 */

static void
Combat_Handle_FreeInternal (Tcl_Obj * obj)
{
  Combat::Object * objInf =
    (Combat::Object *) (void *) obj->internalRep.twoPtrValue.ptr2;

  if (objInf) {
    if (--objInf->refs == 0) {
      Tcl_DeleteCommand (objInf->interp, objInf->name);
      objInf->ctx->active.erase (objInf->name);
      delete objInf;
    }
  }

  Combat::OldCmdType.freeIntRepProc (obj);
}

/*
 * ----------------------------------------------------------------------
 *
 * "Main" function, install our commands in the Tcl interpreter
 *
 * ----------------------------------------------------------------------
 */

#ifdef __MINGW32__
DLLEXPORT
#endif
int
Combat_Init (Tcl_Interp *interp)
{
  Combat::Context * ctx;

#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs (interp, TCL_VERSION, 0) == NULL) {
    return TCL_ERROR;
  }
#else
  if (Tcl_PkgRequire (interp, "Tcl", TCL_VERSION, 1) == NULL) {
    return TCL_ERROR;
  }
#endif

  ctx = new Combat::Context;

  /*
   * Interpreter-specific: add commands
   */

  Tcl_CreateObjCommand (interp, "corba::feature", Combat_Feature,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::init", Combat_Init_Cmd,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::shutdown", Combat_Shutdown,
			(ClientData) ctx, NULL);

  Tcl_CreateObjCommand (interp, "corba::string_to_object",
			Combat_String_To_Object,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::object_to_string",
			Combat_Object_To_String,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::list_initial_services",
			Combat_List_Initial_Services,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::resolve_initial_references",
			Combat_Resolve_Initial_References,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::register_initial_reference",
			Combat_Register_Initial_Reference,
			(ClientData) ctx, NULL);

  Tcl_CreateObjCommand (interp, "corba::dii", Combat_Dii,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::const", Combat_Const,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::type", Combat_Type,
			(ClientData) ctx, NULL);

  Tcl_CreateObjCommand (interp, "corba::throw", Combat_Throw,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::try", Combat_Try,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::request", Combat_Request,
			(ClientData) ctx, NULL);

  Tcl_CreateObjCommand (interp, "corba::duplicate", Combat_Duplicate,
			(ClientData) ctx, NULL);
  Tcl_CreateObjCommand (interp, "corba::release", Combat_Release,
			(ClientData) ctx, NULL);

#ifdef COMBAT_USE_MICO
  Tcl_CreateObjCommand (interp, "mico::bind", Combat_Bind,
			(ClientData) ctx, NULL);
#endif

  /*
   * Combat-specific
   */

#if !defined(COMBAT_NO_SERVER_SIDE)
  Tcl_CreateObjCommand (interp, "combat::servant", Combat_Servant,
			(ClientData) ctx, NULL);
#endif
#if !defined(COMBAT_NO_COMBAT_IR)
  Tcl_CreateObjCommand (interp, "combat::ir", Combat_IR,
			(ClientData) ctx, NULL);
#endif

  /*
   * Initialize global data
   */

  if (Combat::GlobalData == NULL) {
    Combat::GlobalData = new Combat::Global;
    Tcl_CreateExitHandler (Combat_DeleteGlobal, (ClientData) 0);

    /*
     * Register new types only once
     */
    
    Tcl_RegisterObjType (&Combat::TypeCodeType);
    Tcl_RegisterObjType (&Combat::AnyType);

    /*
     * Hijack Tcl's list type
     */

    Combat::ListTypePtr = Tcl_GetObjType ("list");
    memcpy (&Combat::OldListType, Combat::ListTypePtr, sizeof (Tcl_ObjType));
    Combat::ListTypePtr->setFromAnyProc = Combat_ListFromAny;

    /*
     * Hijack Tcl's cmdName type
     */

    Combat::CmdTypePtr = Tcl_GetObjType ("cmdName");
    memcpy (&Combat::OldCmdType, Combat::CmdTypePtr, sizeof (Tcl_ObjType));
    Combat::CmdTypePtr->freeIntRepProc   = Combat_Handle_FreeInternal;
    Combat::CmdTypePtr->dupIntRepProc    = Combat_Handle_DupInternal;
    Combat::CmdTypePtr->updateStringProc = Combat_Handle_UpdateString;
    Combat::CmdTypePtr->setFromAnyProc   = Combat_Handle_SetFromAny;
  }

  /*
   * Add Context Info
   */

  assert (Combat::GlobalData->contexts.find (interp) ==
	  Combat::GlobalData->contexts.end());
  Combat::GlobalData->contexts[interp] = ctx;

#if !defined(COMBAT_NO_SERVER_SIDE)
  /*
   * Try to load [incr Tcl]. If available, configure support for
   * Server-Side scripting. Else ignore the error. This must again
   * be done in each interpreter.
   */

  if (Tcl_PkgRequire (interp, "Itcl", "3.0", 0) != NULL) {
    if (Tcl_GlobalEval (interp, (char *) Combat::PortableServerCode) != TCL_OK) {
      return TCL_ERROR;
    }
    Combat::GlobalData->haveitcl = true;
  }
  else {
    Tcl_ResetResult (interp);
    Combat::GlobalData->haveitcl = false;
  }
#else
  Combat::GlobalData->haveitcl = false;
#endif

  /*
   * Install WhenDeleted handler
   */

  Tcl_CallWhenDeleted (interp, Combat_DeleteLocal, (ClientData) 0);

  /*
   * Ready
   */

  Tcl_PkgProvide (interp, "combat", "0.7");

  return TCL_OK;
}

/*
 * end extern "C"
 */

}
