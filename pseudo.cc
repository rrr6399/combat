/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
/*
 * ----------------------------------------------------------------------
 *
 * Pseudo Object Implementations
 *
 * ----------------------------------------------------------------------
 */

#include "combat.h"
#include <assert.h>

char * combat_pseudo_id = "$Id$";

/*
 * ----------------------------------------------------------------------
 *
 * Combat::PseudoObj base class
 *
 * ----------------------------------------------------------------------
 */

Combat::PseudoObj::PseudoObj ()
{
}

Combat::PseudoObj::~PseudoObj ()
{
}

CORBA::Object_ptr
Combat::PseudoObj::get_managed (void)
{
  return CORBA::Object::_nil ();
}

bool
Combat::PseudoObj::builtin_invoke (Tcl_Interp * interp, Context * ctx,
				   const char * op, int objc,
				   Tcl_Obj *CONST [], Tcl_Obj ** res)
{
  return false;
}

Tcl_Obj *
Combat::PseudoObj::invoke (Tcl_Interp * interp, Context * ctx,
			   const char * opname,
			   int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

  if (builtin_invoke (interp, ctx, opname, objc, objv, &res)) {
    return res;
  }

  return local_invoke (interp, ctx, opname, objc, objv);
}

#if !defined(COMBAT_NO_SERVER_SIDE)

/*
 * ----------------------------------------------------------------------
 *
 * Tcl POA Manager
 *
 * ----------------------------------------------------------------------
 */

Combat::POAManager::POAManager (PortableServer::POAManager_ptr mgr)
{
  managed = PortableServer::POAManager::_duplicate (mgr);
  assert (managed);
}

Combat::POAManager::~POAManager ()
{
  CORBA::release (managed);
}

CORBA::Object_ptr
Combat::POAManager::get_managed (void)
{
  return CORBA::Object::_duplicate (managed);
}

Tcl_Obj *
Combat::POAManager::local_invoke (Tcl_Interp * interp, Context * ctx,
				  const char * opname,
				  int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

#ifdef HAVE_EXCEPTIONS
  try {
#endif

  if (strcmp (opname, "activate") == 0) {
    res = activate (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "hold_requests") == 0) {
    res = hold_requests (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "discard_requests") == 0) {
    res = discard_requests (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "deactivate") == 0) {
    res = deactivate (interp, ctx, objc, objv);
  }
  else {
    Tcl_AppendResult (interp, "error: illegal operation \"", opname,
		      "\" for PortableServer::POAManager", NULL);
    return NULL;
  }

#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
    return NULL;
  }
#endif
  
  return res;
}

Tcl_Obj *
Combat::POAManager::activate (Tcl_Interp * interp, Context * ctx,
			      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"activate\"", NULL);
    return NULL;
  }

  managed->activate ();
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POAManager::hold_requests (Tcl_Interp * interp, Context * ctx,
				   int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"hold_requests wait_for_completion\"", NULL);
    return NULL;
  }

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					    CORBA::_tc_boolean);
  CORBA::Boolean wait_for_completion;

  if (!any) {
    Tcl_AppendResult (interp, "\n  while extracting parameter wait_for_completion for hold_requests", NULL);
    return NULL;
  }

  *any >>= CORBA::Any::to_boolean (wait_for_completion);

  managed->hold_requests (wait_for_completion);

  delete any;
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POAManager::discard_requests (Tcl_Interp * interp, Context * ctx,
				      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"discard_requests wait_for_completion\"", NULL);
    return NULL;
  }

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					    CORBA::_tc_boolean);
  CORBA::Boolean wait_for_completion;

  if (!any) {
    Tcl_AppendResult (interp, "\n  while extracting parameter wait_for_completion for discard_requests", NULL);
    return NULL;
  }

  *any >>= CORBA::Any::to_boolean (wait_for_completion);

  managed->discard_requests (wait_for_completion);

  delete any;
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POAManager::deactivate (Tcl_Interp * interp, Context * ctx,
				int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"deactivate etherealize_objects wait_for_completion\"", NULL);
    return NULL;
  }

  CORBA::Any * a1 = Combat::GetAnyFromObj (interp, ctx, objv[0],
					   CORBA::_tc_boolean);
  CORBA::Any * a2 = Combat::GetAnyFromObj (interp, ctx, objv[1],
					   CORBA::_tc_boolean);

  if (!a1) {
    Tcl_AppendResult (interp, "\n  while extracting parameter etherealize_objects for deactivate", NULL);
    return NULL;
  }

  if (!a2) {
    Tcl_AppendResult (interp, "\n  while extracting parameter wait_for_completion for deactivate", NULL);
    delete a1;
    return NULL;
  }

  CORBA::Boolean etherealize_objects, wait_for_completion;

  *a1 >>= CORBA::Any::to_boolean (etherealize_objects);
  *a2 >>= CORBA::Any::to_boolean (wait_for_completion);

  managed->deactivate (etherealize_objects, wait_for_completion);

  delete a1;
  delete a2;
  return Tcl_NewObj ();
}

/*
 * ----------------------------------------------------------------------
 *
 * Tcl ServerRequest
 *
 * ----------------------------------------------------------------------
 */

Combat::ServerRequest::ServerRequest (CORBA::ServerRequest_ptr req)
{
  managed = CORBA::ServerRequest::_duplicate (req);
  args = CORBA::NVList::_nil();
  assert (managed);
}

Combat::ServerRequest::~ServerRequest ()
{
  CORBA::release (managed);
}

Tcl_Obj *
Combat::ServerRequest::local_invoke (Tcl_Interp * interp, Context * ctx,
				     const char * opname,
				     int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

#ifdef HAVE_EXCEPTIONS
  try {
#endif

  if (strcmp (opname, "operation") == 0) {
    res = operation (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "arguments") == 0) {
    res = arguments (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "set_result") == 0) {
    res = set_result (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "set_exception") == 0) {
    res = set_exception (interp, ctx, objc, objv);
  }
  else {
    Tcl_AppendResult (interp, "error: illegal operation \"", opname,
		      "\" for CORBA::ServerRequest", NULL);
    return NULL;
  }
  
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
    return NULL;
  }
#endif

  return res;
}

Tcl_Obj *
Combat::ServerRequest::operation (Tcl_Interp * interp, Context * ctx,
				  int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"operation\"", NULL);
    return NULL;
  }

  const char * op = managed->operation ();
  return Tcl_NewStringObj ((char *) op, -1);
}

Tcl_Obj *
Combat::ServerRequest::arguments (Tcl_Interp * interp, Context * ctx,
				  int objc, Tcl_Obj *CONST objv[])
{
  int i;

  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"arguments parameters\"", NULL);
    return NULL;
  }

  /*
   * Get parameter list
   */

  int nargs;
  if (Tcl_ListObjLength (NULL, objv[0], &nargs) != TCL_OK) {
    Tcl_AppendResult (interp, "error: not a valid args list: \"",
		      Tcl_GetStringFromObj (objv[0], NULL), "\"", NULL);
    return NULL;
  }

  Combat::GlobalData->orb->create_list (0, args);

  for (i=0; i<nargs; i++) {
    Tcl_Obj *ad, *mode, *type;
    CORBA::TypeCode_var argtype;
    CORBA::Flags argmode;
    const char *argstr;
    int lad;

    if (Tcl_ListObjIndex (NULL, objv[0], i, &ad) != TCL_OK ||
	Tcl_ListObjLength (NULL, ad, &lad) != TCL_OK || lad != 2 ||
	Tcl_ListObjIndex (NULL, ad, 0, &mode) != TCL_OK ||
	Tcl_ListObjIndex (NULL, ad, 1, &type) != TCL_OK) {
      Tcl_AppendResult (interp, "error: not a valid args list: \"",
			Tcl_GetStringFromObj (objv[0], NULL), "\"", NULL);
      CORBA::release (args);
      return NULL;
    }

    argstr = Tcl_GetStringFromObj (mode, NULL);

    if (strncmp (argstr, "CORBA::", 7) == 0) {
      argstr += 7;
    }
    if (strncmp (argstr, "ARG_", 4) == 0 ||
	strncmp (argstr, "arg_", 4) == 0){
      argstr += 4;
    }
    if (strcmp (argstr, "IN") == 0 ||
	strcmp (argstr, "in") == 0) {
      argmode = CORBA::ARG_IN;
    }
    else if (strcmp (argstr, "OUT") == 0 ||
	     strcmp (argstr, "out") == 0) {
      argmode = CORBA::ARG_OUT;
    }
    else if (strcmp (argstr, "INOUT") == 0 ||
	     strcmp (argstr, "inout") == 0) {
      argmode = CORBA::ARG_INOUT;
    }
    else {
      Tcl_AppendResult (interp, "error: not a valid argument mode: \"",
			Tcl_GetStringFromObj (mode, NULL), "\"", NULL);
      CORBA::release (args);
      return NULL;
    }

    argtype = Combat::GetTypeCodeFromObj (interp, type);

    if (CORBA::is_nil (argtype)) {
      Tcl_AppendResult (interp, "\n  while scanning TypeCode from \"",
			Tcl_GetStringFromObj (type, NULL), "\"", NULL);
      CORBA::release (args);
      return NULL;
    }

    CORBA::Any * any = new CORBA::Any (argtype.in(), (void *) NULL);
    args->add_value_consume (CORBA::string_dup (""), any, argmode);
  }

  /*
   * Args are consumed
   */

  managed->arguments (args);

  /*
   * Build result list.
   * For inout / out parameters, we create and pass a variable.
   */

  Tcl_Obj * res = Tcl_NewObj ();
  char tmp[256];

  for (i=0; i<nargs; i++) {
    Tcl_Obj *val;

    if (args->item(i)->flags() == CORBA::ARG_IN) {
      val = Combat::NewAnyObj (interp, ctx, *args->item(i)->value());
      Tcl_ListObjAppendElement (NULL, res, val);
    }
    else if (args->item(i)->flags() == CORBA::ARG_OUT) {
      sprintf (tmp, "_combat_arg_%02d", i);
      Tcl_Obj * name = Tcl_NewStringObj (tmp, -1);
      Tcl_ListObjAppendElement (NULL, res, name);
    }
    else if (args->item(i)->flags() == CORBA::ARG_INOUT) {
      val = Combat::NewAnyObj (interp, ctx, *args->item(i)->value());
      sprintf (tmp, "_combat_arg_%02d", i);
      Tcl_Obj * name = Tcl_NewStringObj (tmp, -1);
      Tcl_ObjSetVar2 (interp, name, NULL, val, 0);
      Tcl_ListObjAppendElement (NULL, res, name);
    }
    else {
      assert (0);
    }
  }

  return res;
}

Tcl_Obj *
Combat::ServerRequest::set_result (Tcl_Interp * interp, Context * ctx,
				   int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"set_result value\"", NULL);
    return NULL;
  }

  /*
   * Set result
   */

  Tcl_Obj * o1, * o2;
  int len;

  if (Tcl_ListObjLength (NULL, objv[0], &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, objv[0], 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, objv[0], 1, &o2) != TCL_OK) {
    Tcl_AppendResult (interp, "error: not an any value: \"",
		      Tcl_GetStringFromObj (objv[0], NULL),
		      "\"", NULL);
    return NULL;
  }

  CORBA::TypeCode_var tc = Combat::GetTypeCodeFromObj (interp, o1);

  if (CORBA::is_nil (tc)) {
    Tcl_AppendResult (interp, "\n while scanning TypeCode from \"",
		      Tcl_GetStringFromObj (o1, NULL),
		      "\"", NULL);
    return NULL;
  }

  CORBA::Any * res = Combat::GetAnyFromObj (interp, ctx, o2, tc.in());

  if (!res) {
    Tcl_AppendResult (interp, "\n  while extracting result value from \"",
		      Tcl_GetStringFromObj (objv[0], NULL), "\"", NULL);
    return NULL;
  }

  managed->set_result (*res);
  delete res;

  /*
   * Push back ARG_OUT and ARG_INOUT parameters
   */

  for (CORBA::ULong i=0; i<args->count(); i++) {
    Tcl_Obj *name, *value;
    char tmp[256];

    if (args->item(i)->flags() == CORBA::ARG_OUT ||
	args->item(i)->flags() == CORBA::ARG_INOUT) {
      sprintf (tmp, "_combat_arg_%02lu", i);
      name  = Tcl_NewStringObj (tmp, -1);
      value = Tcl_ObjGetVar2 (interp, name, NULL, TCL_LEAVE_ERR_MSG);
      Tcl_DecrRefCount (name);

      if (value == NULL) {
	Tcl_AppendResult (interp, "\n after invoking \"",
			  managed->operation(), "\"", NULL);
	Tcl_BackgroundError (interp);
	CORBA::Any ex;
	ex <<= CORBA::INTERNAL (0, CORBA::COMPLETED_YES);
	managed->set_exception (ex);
	return NULL;
      }

      CORBA::TypeCode_var tc = args->item(i)->value()->type();
      CORBA::Any * par = Combat::GetAnyFromObj (interp, ctx, value, tc.in());
      Tcl_UnsetVar (interp, tmp, 0);
      
      if (!par) {
	Tcl_AppendResult (interp, "\n after invoking \"",
			  managed->operation(), "\"", NULL);
	Tcl_BackgroundError (interp);
	CORBA::Any ex;
	ex <<= CORBA::MARSHAL (0, CORBA::COMPLETED_YES);
	managed->set_exception (ex);
	return NULL;
      }
      
      *(args->item(i)->value()) = *par;
      delete par;
    }
  }

  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::ServerRequest::set_exception (Tcl_Interp * interp, Context * ctx,
				       int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"set_exception value\"", NULL);
    return NULL;
  }

  Tcl_Obj *o1, *o2;
  int len;

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					    CORBA::_tc_any);
  
  if (any == NULL) {
    Tcl_AppendResult (interp, "\n  while setting exception from \"",
		      Tcl_GetStringFromObj (objv[0], NULL), "\"", NULL);
    return NULL;
  }

  const CORBA::Any *ex;
  *any >>= ex;

  managed->set_exception (*ex);
  delete any;

  return Tcl_NewObj ();
}

/*
 * ----------------------------------------------------------------------
 *
 * Tcl POA
 *
 * ----------------------------------------------------------------------
 */

Combat::POA::POA (PortableServer::POA_ptr poa)
{
  managed = PortableServer::POA::_duplicate (poa);
  assert (managed);
}

Combat::POA::~POA ()
{
  CORBA::release (managed);
}

CORBA::Object_ptr
Combat::POA::get_managed (void)
{
  return CORBA::Object::_duplicate (managed);
}

Tcl_Obj *
Combat::POA::local_invoke (Tcl_Interp * interp, Context * ctx,
			   const char * opname,
			   int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

#ifdef HAVE_EXCEPTIONS
  try {
#endif

  if (strcmp (opname, "create_POA") == 0) {
    res = create_POA (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "find_POA") == 0) {
    res = find_POA (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "destroy") == 0) {
    res = destroy (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "the_name") == 0) {
    res = the_name (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "the_parent") == 0) {
    res = the_parent (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "the_POAManager") == 0) {
    res = the_POAManager (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "the_activator") == 0) {
    res = the_activator (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "get_servant_manager") == 0) {
    res = get_servant_manager (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "set_servant_manager") == 0) {
    res = set_servant_manager (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "get_servant") == 0) {
    res = get_servant (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "set_servant") == 0) {
    res = set_servant (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "activate_object") == 0) {
    res = activate_object (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "activate_object_with_id") == 0) {
    res = activate_object_with_id (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "deactivate_object") == 0) {
    res = deactivate_object (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "create_reference") == 0) {
    res = create_reference (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "create_reference_with_id") == 0) {
    res = create_reference_with_id (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "servant_to_id") == 0) {
    res = servant_to_id (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "servant_to_reference") == 0) {
    res = servant_to_reference (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "reference_to_servant") == 0) {
    res = reference_to_servant (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "reference_to_id") == 0) {
    res = reference_to_id (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "id_to_servant") == 0) {
    res = id_to_servant (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "id_to_reference") == 0) {
    res = id_to_reference (interp, ctx, objc, objv);
  }
  else {
    Tcl_AppendResult (interp, "error: illegal operation \"", opname,
		      "\" for PortableServer::POA", NULL);
    return NULL;
  }

#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
    return NULL;
  }
#endif
  
  return res;
}

Tcl_Obj *
Combat::POA::create_POA (Tcl_Interp * interp, Context * ctx,
			 int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 3) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"create_POA adapter_name a_POAManager policies\"", NULL);
    return NULL;
  }

  const char * adapter_name = Tcl_GetStringFromObj (objv[0], NULL);
  const char * pmname = Tcl_GetStringFromObj (objv[1], NULL);
  PortableServer::POAManager_var a_POAManager;

  if (strcmp (pmname, "") != 0 && strcmp (pmname, "0") != 0) {
    Tcl_CmdInfo info;
    if (!Tcl_GetCommandInfo (interp, (char *) pmname, &info)) {
      Tcl_AppendResult (interp, "error: no such object: \"",
			pmname, "\"", NULL);
      return NULL;
    }

    Combat::Object * mobj = (Combat::Object *) info.objClientData;
    CORBA::Object_var obj;

    if (mobj->pseudo) {
      obj = mobj->pseudo->get_managed ();
    }
    else {
      obj = CORBA::Object::_nil ();
    }

    a_POAManager = PortableServer::POAManager::_narrow (obj);

    if (CORBA::is_nil (a_POAManager)) {
      Tcl_AppendResult (interp, "error: \"", pmname, "\" is not a ",
			"PortableServer::POAManager object", NULL);
      return NULL;
    }
  }
  else {
    a_POAManager = PortableServer::POAManager::_nil ();
  }

  int npols;
  if (Tcl_ListObjLength (NULL, objv[2], &npols) != TCL_OK) {
    Tcl_AppendResult (interp, "error: expecting list of policies, got \"",
		      Tcl_GetStringFromObj (objv[2], NULL), "\"", NULL);
    return NULL;
  }

  CORBA::PolicyList policies (npols);
  policies.length (npols);

  for (int i=0; i<npols; i++) {
    const char * pname;
    Tcl_Obj * pobj;

    Tcl_ListObjIndex (NULL, objv[2], i, &pobj);
    pname = Tcl_GetStringFromObj (pobj, NULL);

    if (strncmp (pname, "PortableServer::", 16) == 0) {
      pname += 16;
    }

    if (strcmp (pname, "ORB_CTRL_MODEL") == 0) {
      policies[i] = managed->create_thread_policy (PortableServer::ORB_CTRL_MODEL);
    }
    else if (strcmp (pname, "SINGLE_THREAD_MODEL") == 0) {
      policies[i] = managed->create_thread_policy (PortableServer::SINGLE_THREAD_MODEL);
    }
    else if (strcmp (pname, "MAIN_THREAD_MODEL") == 0) {
      policies[i] = managed->create_thread_policy (PortableServer::MAIN_THREAD_MODEL);
    }
    else if (strcmp (pname, "TRANSIENT") == 0) {
      policies[i] = managed->create_lifespan_policy (PortableServer::TRANSIENT);
    }
    else if (strcmp (pname, "PERSISTENT") == 0) {
      policies[i] = managed->create_lifespan_policy (PortableServer::PERSISTENT);
    }
    else if (strcmp (pname, "UNIQUE_ID") == 0) {
      policies[i] = managed->create_id_uniqueness_policy (PortableServer::UNIQUE_ID);
    }
    else if (strcmp (pname, "MULTIPLE_ID") == 0) {
      policies[i] = managed->create_id_uniqueness_policy (PortableServer::MULTIPLE_ID);
    }
    else if (strcmp (pname, "USER_ID") == 0) {
      policies[i] = managed->create_id_assignment_policy (PortableServer::USER_ID);
    }
    else if (strcmp (pname, "SYSTEM_ID") == 0) {
      policies[i] = managed->create_id_assignment_policy (PortableServer::SYSTEM_ID);
    }
    else if (strcmp (pname, "IMPLICIT_ACTIVATION") == 0) {
      policies[i] = managed->create_implicit_activation_policy (PortableServer::IMPLICIT_ACTIVATION);
    }
    else if (strcmp (pname, "NO_IMPLICIT_ACTIVATION") == 0) {
      policies[i] = managed->create_implicit_activation_policy (PortableServer::NO_IMPLICIT_ACTIVATION);
    }
    else if (strcmp (pname, "RETAIN") == 0) {
      policies[i] = managed->create_servant_retention_policy (PortableServer::RETAIN);
    }
    else if (strcmp (pname, "NON_RETAIN") == 0) {
      policies[i] = managed->create_servant_retention_policy (PortableServer::NON_RETAIN);
    }
    else if (strcmp (pname, "USE_ACTIVE_OBJECT_MAP_ONLY") == 0) {
      policies[i] = managed->create_request_processing_policy (PortableServer::USE_ACTIVE_OBJECT_MAP_ONLY);
    }
    else if (strcmp (pname, "USE_DEFAULT_SERVANT") == 0) {
      policies[i] = managed->create_request_processing_policy (PortableServer::USE_DEFAULT_SERVANT);
    }
    else if (strcmp (pname, "USE_SERVANT_MANAGER") == 0) {
      policies[i] = managed->create_request_processing_policy (PortableServer::USE_SERVANT_MANAGER);
    }
    else {
      Tcl_AppendResult (interp, "error: illegal policy: \"", pname,
			"\"", NULL);
      return NULL;
    }
  }

  PortableServer::POA_var poa = managed->create_POA (adapter_name,
						     a_POAManager,
						     policies);
  Combat::POA * mpoa = new Combat::POA (poa);
  return Combat::InstantiateObj (interp, ctx, mpoa);
}

Tcl_Obj *
Combat::POA::find_POA (Tcl_Interp * interp, Context * ctx,
		       int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"find_POA adapter_name activate_it\"", NULL);
    return NULL;
  }

  const char * adapter_name = Tcl_GetStringFromObj (objv[0], NULL);
  CORBA::Any * a1 = Combat::GetAnyFromObj (interp, ctx, objv[1],
					   CORBA::_tc_boolean);
  CORBA::Boolean activate_it;

  if (!a1) {
    Tcl_AppendResult (interp, "\n  while extracting parameter activate_it for find_POA", NULL);
    return NULL;
  }

  *a1 >>= CORBA::Any::to_boolean (activate_it);

  PortableServer::POA_var poa = managed->find_POA (adapter_name,
						   activate_it);

  delete a1;

  if (CORBA::is_nil (poa)) {
    Tcl_AppendResult (interp, "error: adapter \"", adapter_name,
		      "\" does not exist in this POA", NULL);
    return NULL;
  }

  Combat::POA * mpoa = new Combat::POA (poa);
  return Combat::InstantiateObj (interp, ctx, mpoa);
}

Tcl_Obj *
Combat::POA::destroy (Tcl_Interp * interp, Context * ctx,
		      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"destroy etherealize_objects wait_for_completion\"", NULL);
    return NULL;
  }

  CORBA::Any * a1 = Combat::GetAnyFromObj (interp, ctx, objv[0],
					   CORBA::_tc_boolean);
  CORBA::Any * a2 = Combat::GetAnyFromObj (interp, ctx, objv[1],
					   CORBA::_tc_boolean);

  if (!a1) {
    Tcl_AppendResult (interp, "\n  while extracting parameter etherealize_objects for destroy", NULL);
    return NULL;
  }

  if (!a2) {
    Tcl_AppendResult (interp, "\n  while extracting parameter wait_for_completion for destroy", NULL);
    delete a1;
    return NULL;
  }

  CORBA::Boolean etherealize_objects, wait_for_completion;

  *a1 >>= CORBA::Any::to_boolean (etherealize_objects);
  *a2 >>= CORBA::Any::to_boolean (wait_for_completion);

  managed->destroy (etherealize_objects, wait_for_completion);

  delete a1;
  delete a2;
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POA::the_name (Tcl_Interp * interp, Context * ctx,
		       int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"the_name\"", NULL);
    return NULL;
  }

  CORBA::String_var name = managed->the_name ();
  return Tcl_NewStringObj ((char *) name.in(), -1);
}

Tcl_Obj *
Combat::POA::the_parent (Tcl_Interp * interp, Context * ctx,
			  int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"the_parent\"", NULL);
    return NULL;
  }

  PortableServer::POA_var parent = managed->the_parent ();

  if (CORBA::is_nil (parent)) {
    Tcl_AppendResult (interp, "error: the RootPOA does not have a parent", NULL);
    return NULL;
  }

  Combat::POA * mpoa = new Combat::POA (parent);
  return Combat::InstantiateObj (interp, ctx, mpoa);
}

Tcl_Obj *
Combat::POA::the_POAManager (Tcl_Interp * interp, Context * ctx,
			     int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"the_POAManager\"", NULL);
    return NULL;
  }

  PortableServer::POAManager_var manager = managed->the_POAManager ();
  Combat::POAManager * mmgr = new Combat::POAManager (manager);
  return Combat::InstantiateObj (interp, ctx, mmgr);
}

Tcl_Obj *
Combat::POA::the_activator (Tcl_Interp * interp, Context * ctx,
			    int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

  if (objc != 0 && objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"the_activator ?adapter_activator?\"", NULL);
    return NULL;
  }

  if (objc == 0) {
    PortableServer::AdapterActivator_ptr aa = managed->the_activator ();

    if (CORBA::is_nil (aa)) {
      res = Tcl_NewIntObj (0);
      return res;
    }

    if ((res = Combat::InstantiateObj (interp, ctx, aa)) == NULL) {
      CORBA::release (aa);
      return NULL;
    }
  }
  else {
    CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					      CORBA::_tc_Object);
    CORBA::Object_var obj;

    if (!any) {
      return NULL;
    }
  
    *any >>= CORBA::Any::to_object (obj);

    PortableServer::AdapterActivator_var aa =
      PortableServer::AdapterActivator::_narrow (obj);

    delete any;

    if (CORBA::is_nil (aa)) {
      Tcl_AppendResult (interp, "error: not an AdapterActivator: \"",
			Tcl_GetStringFromObj (objv[0], NULL), "\"", NULL);
      return NULL;
    }

    managed->the_activator (aa);
    res = Tcl_NewObj ();
  }

  return res;
}

Tcl_Obj *
Combat::POA::get_servant_manager (Tcl_Interp * interp, Context * ctx,
				  int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"get_servant_manager\"", NULL);
    return NULL;
  }

  PortableServer::ServantManager_ptr mgr = managed->get_servant_manager ();

  if (CORBA::is_nil (mgr)) {
    return Tcl_NewIntObj (0);
  }

  if ((res = Combat::InstantiateObj (interp, ctx, mgr)) == NULL) {
    CORBA::release (mgr);
    return NULL;
  }
  
  return res;
}

Tcl_Obj *
Combat::POA::set_servant_manager (Tcl_Interp * interp, Context * ctx,
				  int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"set_servant_manager imgr\"", NULL);
    return NULL;
  }

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					    CORBA::_tc_Object);
  CORBA::Object_var obj;

  if (!any) {
    return NULL;
  }
  
  *any >>= CORBA::Any::to_object (obj);

  PortableServer::ServantManager_var mgr =
    PortableServer::ServantManager::_narrow (obj);

  delete any;

  if (CORBA::is_nil (mgr)) {
    Tcl_AppendResult (interp, "error: not a ServantManager: \"",
		      Tcl_GetStringFromObj (objv[0], NULL), "\"", NULL);
    return NULL;
  }

  managed->set_servant_manager (mgr);
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POA::get_servant (Tcl_Interp * interp, Context * ctx,
			   int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"get_servant\"", NULL);
    return NULL;
  }

  PortableServer::Servant serv = managed->get_servant ();

  if (serv == NULL) {
    Tcl_AppendResult (interp, "error: no default servant registered with this POA", NULL);
    return NULL;
  }

  /*
   * search the Servant in our map
   */

  Combat::Context::ServantMap::iterator it = ctx->servants.begin();
  Tcl_Obj * thename = NULL;

  while (it != ctx->servants.end()) {
    if (serv == (*it).second) {
      thename = Tcl_NewStringObj ((char *) (*it).first.c_str(), -1);
      break;
    }
    it++;
  }

  if (!thename) {
    Tcl_AppendResult (interp, "error: oops: servant not found ",
		      "for get_servant", NULL);
    return NULL;
  }

  return thename;
}

Tcl_Obj *
Combat::POA::set_servant (Tcl_Interp * interp, Context * ctx,
			  int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"set_servant p_servant\"", NULL);
    return NULL;
  }

  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[0]);

  if (serv == NULL) {
    return NULL;
  }
  
  managed->set_servant (serv);
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POA::activate_object (Tcl_Interp * interp, Context * ctx,
			      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"activate_object p_servant\"", NULL);
    return NULL;
  }

  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[0]);

  if (serv == NULL) {
    return NULL;
  }

  PortableServer::ObjectId_var id = managed->activate_object (serv);
  return Combat::ObjectId_to_Tcl_Obj (*id);
}

Tcl_Obj *
Combat::POA::activate_object_with_id (Tcl_Interp * interp, Context * ctx,
				      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"activate_object_with_id id p_servant\"", NULL);
    return NULL;
  }

  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[1]);

  if (serv == NULL) {
    return NULL;
  }

  PortableServer::ObjectId_var id = Combat::Tcl_Obj_to_ObjectId (objv[0]);
  managed->activate_object_with_id (*id, serv);
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POA::deactivate_object (Tcl_Interp * interp, Context * ctx,
				int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"deactivate_object oid\"", NULL);
    return NULL;
  }

  PortableServer::ObjectId_var id = Combat::Tcl_Obj_to_ObjectId (objv[0]);
  managed->deactivate_object (*id);
  return Tcl_NewObj ();
}

Tcl_Obj *
Combat::POA::create_reference (Tcl_Interp * interp, Context * ctx,
			       int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"create_reference intf\"", NULL);
    return NULL;
  }

  const char * repoid = Tcl_GetStringFromObj (objv[0], NULL);
  CORBA::Object_ptr obj = managed->create_reference (repoid);
  return Combat::InstantiateObj (interp, ctx, obj);
}

Tcl_Obj *
Combat::POA::create_reference_with_id (Tcl_Interp * interp, Context * ctx,
				       int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 2) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"create_reference_with_id oid intf\"", NULL);
    return NULL;
  }

  const char * repoid = Tcl_GetStringFromObj (objv[1], NULL);
  PortableServer::ObjectId_var id = Combat::Tcl_Obj_to_ObjectId (objv[0]);
  CORBA::Object_ptr obj = managed->create_reference_with_id (*id, repoid);
  return Combat::InstantiateObj (interp, ctx, obj);
}

Tcl_Obj *
Combat::POA::servant_to_id (Tcl_Interp * interp, Context * ctx,
			    int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"servant_to_id p_servant\"", NULL);
    return NULL;
  }

  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[0]);

  if (serv == NULL) {
    return NULL;
  }
  
  PortableServer::ObjectId_var id = managed->servant_to_id (serv);
  return Combat::ObjectId_to_Tcl_Obj (*id);
}

Tcl_Obj *
Combat::POA::servant_to_reference (Tcl_Interp * interp, Context * ctx,
				   int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"servant_to_reference p_servant\"", NULL);
    return NULL;
  }

  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, objv[0]);

  if (serv == NULL) {
    return NULL;
  }

  CORBA::Object_ptr obj = managed->servant_to_reference (serv);
  return Combat::InstantiateObj (interp, ctx, obj);
}

Tcl_Obj *
Combat::POA::reference_to_servant (Tcl_Interp * interp, Context * ctx,
				   int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"reference_to_servant reference\"", NULL);
    return NULL;
  }

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					    CORBA::_tc_Object);
  CORBA::Object_var obj;

  if (!any) {
    return NULL;
  }
  
  *any >>= CORBA::Any::to_object (obj);

  PortableServer::Servant serv = managed->reference_to_servant (obj);

  delete any;

  /*
   * search the Servant in our map
   */

  Combat::Context::ServantMap::iterator it = ctx->servants.begin();
  Tcl_Obj * thename = NULL;

  while (it != ctx->servants.end()) {
    if (serv == (*it).second) {
      thename = Tcl_NewStringObj ((char *) (*it).first.c_str(), -1);
      break;
    }
    it++;
  }

  if (!thename) {
    Tcl_AppendResult (interp, "error: oops: servant not found ",
		      "for reference_to_servant", NULL);
    return NULL;
  }

  return thename;
}

Tcl_Obj *
Combat::POA::reference_to_id (Tcl_Interp * interp, Context * ctx,
			      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"reference_to_id reference\"", NULL);
    return NULL;
  }

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					    CORBA::_tc_Object);
  CORBA::Object_var obj;

  if (!any) {
    return NULL;
  }
  
  *any >>= CORBA::Any::to_object (obj);

  PortableServer::ObjectId_var id = managed->reference_to_id (obj);
  delete any;
  return Combat::ObjectId_to_Tcl_Obj (*id);
}

Tcl_Obj *
Combat::POA::id_to_servant (Tcl_Interp * interp, Context * ctx,
			    int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"id_to_servant oid\"", NULL);
    return NULL;
  }

  PortableServer::ObjectId_var id = Combat::Tcl_Obj_to_ObjectId (objv[0]);
  PortableServer::Servant serv = managed->id_to_servant (*id);

  /*
   * search the Servant in our map
   */

  Combat::Context::ServantMap::iterator it = ctx->servants.begin();
  Tcl_Obj * thename = NULL;

  while (it != ctx->servants.end()) {
    if (serv == (*it).second) {
      thename = Tcl_NewStringObj ((char *) (*it).first.c_str(), -1);
      break;
    }
    it++;
  }

  if (!thename) {
    Tcl_AppendResult (interp, "error: oops: servant not found ",
		      "for id_to_servant", NULL);
    return NULL;
  }

  return thename;
}

Tcl_Obj *
Combat::POA::id_to_reference (Tcl_Interp * interp, Context * ctx,
			      int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 1) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"id_to_reference oid\"", NULL);
    return NULL;
  }

  PortableServer::ObjectId_var id = Combat::Tcl_Obj_to_ObjectId (objv[0]);
  CORBA::Object_ptr obj = managed->id_to_reference (*id);
  return Combat::InstantiateObj (interp, ctx, obj);
}

/*
 * ----------------------------------------------------------------------
 *
 * Tcl POACurrent
 *
 * ----------------------------------------------------------------------
 */

Combat::POACurrent::POACurrent (PortableServer::Current_ptr current)
{
  managed = PortableServer::Current::_duplicate (current);
  assert (managed);
}

Combat::POACurrent::~POACurrent ()
{
  CORBA::release (managed);
}

CORBA::Object_ptr
Combat::POACurrent::get_managed (void)
{
  return CORBA::Object::_duplicate (managed);
}

Tcl_Obj *
Combat::POACurrent::local_invoke (Tcl_Interp * interp, Context * ctx,
				  const char * opname,
				  int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj * res;

#ifdef HAVE_EXCEPTIONS
  try {
#endif

  if (strcmp (opname, "get_POA") == 0) {
    res = get_POA (interp, ctx, objc, objv);
  }
  else if (strcmp (opname, "get_object_id") == 0) {
    res = get_object_id (interp, ctx, objc, objv);
  }
  else {
    Tcl_AppendResult (interp, "error: illegal operation \"", opname,
		      "\" for PortableServer::Current", NULL);
    return NULL;
  }

#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
    return NULL;
  }
#endif
  
  return res;
}

Tcl_Obj *
Combat::POACurrent::get_POA (Tcl_Interp * interp, Context * ctx,
			     int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"get_POA\"", NULL);
    return NULL;
  }

  PortableServer::POA_ptr poa = managed->get_POA ();
  Combat::POA * mpoa = new Combat::POA (poa);
  return Combat::InstantiateObj (interp, ctx, mpoa);
}

Tcl_Obj *
Combat::POACurrent::get_object_id (Tcl_Interp * interp, Context * ctx,
				   int objc, Tcl_Obj *CONST objv[])
{
  if (objc != 0) {
    Tcl_AppendResult (interp, "error: wrong # args: should be \"get_object_id\"", NULL);
    return NULL;
  }

  PortableServer::ObjectId_var oid = managed->get_object_id ();
  return Combat::ObjectId_to_Tcl_Obj (*oid);
}

/*
 * if defined(COMBAT_NO_SERVER_SIDE)
 */

#endif

