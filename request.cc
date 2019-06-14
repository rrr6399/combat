/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
#include "combat.h"
#include <string>
#include <assert.h>

char * combat_request_id = "$Id$";

/*
 * ----------------------------------------------------------------------
 *
 * Request handle base class implementation. Not much here.
 *
 * ----------------------------------------------------------------------
 */

#ifdef HAVE_NAMESPACE
namespace Combat {
  UniqueIdGenerator Request::IdFactory("_mico_req_");
};
#else
Combat::UniqueIdGenerator Combat::Request::IdFactory ("_mico_req_");
#endif

Combat::Request::Request ()
{
  cbinterp = NULL;
  cbfunc = NULL;
  id = IdFactory.new_id();
}

Combat::Request::~Request ()
{
  if (cbfunc) {
    Tcl_DecrRefCount (cbfunc);
  }
}

const char *
Combat::Request::get_id () const
{
  return id.in();
}

/*
 * Callback into Tcl
 */

void
Combat::Request::SetCallback (Tcl_Interp * interp, Tcl_Obj * func)
{
  cbinterp = interp;
  cbfunc = func;
  Tcl_IncrRefCount (cbfunc);
}

void
Combat::Request::PerformCallback ()
{
  assert (cbinterp);
  assert (cbfunc);

  /*
   * The request will probably be deleted as a side effect of calling
   * the callback script. Remember not to touch any members afterwards.
   */

  Tcl_Interp * myinterp = cbinterp;

  Tcl_Obj *o[2], *com;
  o[0] = cbfunc;
  o[1] = Tcl_NewStringObj ((char *) id.in(), -1);
  com  = Tcl_NewListObj (2, o);
  Tcl_IncrRefCount (com);

  if (Tcl_GlobalEvalObj (myinterp, com) != TCL_OK) {
    Tcl_BackgroundError (myinterp);
  }
  Tcl_DecrRefCount (com);
}

/*
 * ----------------------------------------------------------------------
 *
 * Object Request implementation.
 *
 * ----------------------------------------------------------------------
 */

Combat::ObjectRequest::ObjectRequest (Object * _o)
{
  obj = _o;
  pds = NULL;
  params = NULL;
  is_builtin = false;
  is_oneway = false;
  is_finished = false;
  builtin_result = NULL;
  req_except = NULL;
}

Combat::ObjectRequest::~ObjectRequest ()
{
  if (params && pds) {
    for (CORBA::ULong i=0; i < pds->length(); i++) {
      Tcl_DecrRefCount (params[i]);
    }
    delete [] params;
  }

  delete pds;

  if (is_builtin && builtin_result) {
    Tcl_DecrRefCount (builtin_result);
  }
  if (req_except) {
    Tcl_DecrRefCount (req_except);
  }
}

/*
 * get attribute request
 */

int
Combat::ObjectRequest::SetupGet (Tcl_Interp * interp,
				 const char * attr,
				 CORBA::AttributeDescription * ad)
{
  std::string command ("_get_");
  command += attr;
  req      = obj->obj->_request (command.c_str());
  rtype    = CORBA::TypeCode::_duplicate (ad->type);
  req->set_return_type (rtype.in());
  return TCL_OK;
}

/*
 * set attribute request
 */

int
Combat::ObjectRequest::SetupSet (Tcl_Interp * interp,
				 const char * attr,
				 Tcl_Obj * data,
				 CORBA::AttributeDescription * ad)
{
  std::string command = ("_set_");
  command += attr;

  if (ad->mode == CORBA::ATTR_READONLY) {
    Tcl_AppendResult (interp, "error: attribute \"", attr,
		      "\" is readonly", NULL);
    return TCL_ERROR;
  }

  CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, data, ad->type.in());

  if (!any) {
    Tcl_AppendResult (interp, "\n  while setting attribute \"", attr,
		      "\" from \"", Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

#if !defined(COMBAT_USE_ORBIX)
  rtype = CORBA::TypeCode::_duplicate (CORBA::_tc_void);
#else
  rtype = CORBA::TypeCode::_nil ();
#endif
  req   = obj->obj->_request (command.c_str());
  req->arguments()->add_value_consume (CORBA::string_dup (""),
				       any, CORBA::ARG_IN);
  req->set_return_type (CORBA::_tc_void);

  return TCL_OK;
}

/*
 * method invocation request
 *
 * objv[0] is the first actual parameter, not the object name
 */

int
Combat::ObjectRequest::SetupInvoke (Tcl_Interp * interp, const char * op,
				    int objc, Tcl_Obj *CONST objv[],
				    CORBA::OperationDescription * od)
{
  CORBA::ULong i;
  int res;

  res = TCL_OK;
  pds = new CORBA::ParDescriptionSeq (od->parameters);

  if ((CORBA::ULong) objc != od->parameters.length()) {
    char tmp[64];
    sprintf (tmp, "%lu", (unsigned long) od->parameters.length());
    Tcl_AppendResult (interp, "error: operation \"", op,
		      "\" of interface ", obj->iface->id(),
		      " takes ", tmp, " parameters", NULL);
    return TCL_ERROR;
  }

  /*
   * Build request
   */

  req   = obj->obj->_request (op);
  rtype = CORBA::TypeCode::_duplicate (od->result);
  req->set_return_type (rtype.in());

  if (od->mode == CORBA::OP_ONEWAY) {
    is_oneway = true;
  }

  /*
   * User Exceptions
   */

  for (i=0; i < od->exceptions.length(); i++) {
    req->exceptions()->add (od->exceptions[i].type);
  }
  
  /*
   * Parameters
   */

  for (i=0; i < od->parameters.length(); i++) {
    CORBA::Any * any;
    CORBA::Flags mode;

    switch (od->parameters[i].mode) {
    case CORBA::PARAM_IN:
      any = Combat::GetAnyFromObj (interp, ctx, objv[i],
				    od->parameters[i].type);
      mode = CORBA::ARG_IN;
      break;

    case CORBA::PARAM_OUT:
      any = new CORBA::Any (od->parameters[i].type, (void *) NULL);
      mode = CORBA::ARG_OUT;
      break;

    case CORBA::PARAM_INOUT:
      {
	Tcl_Obj * data;

	if ((data = Tcl_ObjGetVar2 (interp, objv[i], NULL,
				    TCL_PARSE_PART1)) == NULL) {
	  Tcl_AppendResult (interp, "can't read \"",
			    Tcl_GetStringFromObj (objv[i], NULL),
			    "\": no such variable", NULL);
	  any = NULL;
	  break;
	}
	any = Combat::GetAnyFromObj (interp, ctx, data,
				      od->parameters[i].type);
	mode = CORBA::ARG_INOUT;
      }
      break;

    default:
      assert (0);
    }

    if (!any) {
      Tcl_AppendResult (interp, "\n  while packing parameter ",
			od->parameters[i].name.in(),
			" of operation \"", op, "\"", NULL);
      return TCL_ERROR;
    }

    req->arguments()->add_value_consume (CORBA::string_dup (""), any, mode);
  }

  /*
   * Save information about out/inout parameters
   */

  params = new Tcl_Obj * [od->parameters.length()];

  for (i=0; i < od->parameters.length(); i++) {
    Tcl_IncrRefCount (objv[i]);
    params[i] = objv[i];
  }

  return TCL_OK;
}

/*
 * Invocations on pseudo objects
 */

bool
Combat::ObjectRequest::SetupPseudo (Tcl_Interp * interp, const char * op,
				    int objc, Tcl_Obj *CONST objv[],
				    int * res)
{
  if (obj->pseudo) {
    builtin_result = obj->pseudo->invoke (interp, ctx, op, objc, objv);
    if (builtin_result == NULL) {
      *res = TCL_ERROR;
    }
    else {
      is_builtin = true;
      Tcl_IncrRefCount (builtin_result);
      *res = TCL_OK;
    }
    return true;
  }

  return false;
}

/*
 * Builtin Invocations (on "real objects")
 */

bool
Combat::ObjectRequest::SetupBuiltin (Tcl_Interp * interp, const char * op,
				     int objc, Tcl_Obj *CONST objv[],
				     int * res)
{
  if (CORBA::is_nil (obj->obj)) {
    return false;
  }

  if (strcmp (op, "_get_interface") == 0) {
    if (objc != 0) {
      Tcl_AppendResult (interp, "error: operation \"", op,
			"\" takes 0 parameters", NULL);
      is_builtin = true;
      *res = TCL_ERROR;
      return true;
    }

    CORBA::InterfaceDef_ptr ifd;
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      ifd = obj->obj->_get_interface ();
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }
#endif

    assert (!CORBA::is_nil (ifd));

    /*
     * Update our type information
     */

    obj->UpdateType (ifd);

    builtin_result = Combat::InstantiateObj (interp, ctx, ifd);

    assert (builtin_result != NULL);
    *res = TCL_OK;
  }
  else if (strcmp (op, "_is_a") == 0) {
    if (objc != 1) {
      Tcl_AppendResult (interp, "error: operation \"", op,
			"\" takes 1 parameter", NULL);
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }
    const char * repoid = Tcl_GetStringFromObj (objv[0], NULL);
    CORBA::Boolean result;
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      result = obj->obj->_is_a (repoid);
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }
#endif

    builtin_result = Tcl_NewBooleanObj ((result) ? 1 : 0);
    if (result) {
      obj->UpdateType (repoid);
    }
    *res = TCL_OK;
  }
  else if (strcmp (op, "_non_existent") == 0) {
    if (objc != 0) {
      Tcl_AppendResult (interp, "error: operation \"", op,
			"\" takes 0 parameters", NULL);
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }
    CORBA::Boolean result;
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      result = obj->obj->_non_existent ();
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }
#endif
    builtin_result = Tcl_NewBooleanObj ((result) ? 1 : 0);
    *res = TCL_OK;
  }
  else if (strcmp (op, "_is_equivalent") == 0) {
    if (objc != 1) {
      Tcl_AppendResult (interp, "error: operation \"", op,
			"\" takes 1 parameter", NULL);
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }

    CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, objv[0],
					       CORBA::_tc_Object);
    CORBA::Object_var parobj;

    if (!any) {
      Tcl_AppendResult (interp, "error: not an object: \"",
			Tcl_GetStringFromObj (objv[0], NULL),
			"\"", NULL);
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }

    *any >>= CORBA::Any::to_object (parobj);
    delete any;

    CORBA::Boolean result;
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      result = obj->obj->_is_equivalent (parobj);
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
      *res = TCL_ERROR;
      is_builtin = true;
      return true;
    }
#endif
    builtin_result = Tcl_NewBooleanObj ((result) ? 1 : 0);
    *res = TCL_OK;
  }
  else if (strcmp (op, "_duplicate") == 0) {
    builtin_result =
      Combat::InstantiateObj (interp, ctx,
			      CORBA::Object::_duplicate (obj->obj));
    *res = TCL_OK;
  }
  else {
    return false;
  }

  Tcl_IncrRefCount (builtin_result);

  is_builtin = true;
  return true;
}

/*
 * Setup an invocation
 */

int
Combat::ObjectRequest::Setup (Tcl_Interp * interp, Context * c,
			      int objc, Tcl_Obj *CONST objv[])
{
  int res;

  assert (obj);
  ctx = c;

  /*
   * objv[0] is operation name, objv[1] is the first actual parameter.
   */

  const char * op = Tcl_GetStringFromObj (objv[0], NULL);
  objv++; objc--;

  /*
   * Handle Invocations on Pseudo Objects
   */

  if (SetupPseudo (interp, op, objc, objv, &res)) {
    return res;
  }

  /*
   * Handle Builtins
   */

  if (SetupBuiltin (interp, op, objc, objv, &res)) {
    return res;
  }

  /*
   * Do we have type information for the object?
   */

  if (obj->iface == NULL) {
    if (!obj->UpdateType()) {
      Tcl_AppendResult (interp, "error: no type information for \"",
			obj->name, "\": _get_interface failed",
			NULL);
      return TCL_ERROR;
    }
  }

  assert (obj->iface);

  /*
   * Operation or Attribute?
   */

  CORBA::OperationDescription * od;
  CORBA::AttributeDescription * ad;
  bool found;

  /*
   * If lookup failed, update our type information, maybe it's wrong, or
   * maybe the object has morphed.
   */

  if (!(found = obj->iface->lookup (op, od, ad))) {
    if (obj->UpdateType()) {
      found = obj->iface->lookup (op, od, ad);
    }
  }

  if (!found) {
    Tcl_AppendResult (interp, "error: \"", op,
		      "\" is neither operation nor attribute for \"",
		      obj->iface->id(), "\"", NULL);
    return TCL_ERROR;
  }

  assert (od != NULL || ad != NULL);

  if (od != NULL) {
    res = SetupInvoke (interp, op, objc, objv, od);
  }
  else {
    if (objc == 0) {
      res = SetupGet (interp, op, ad);
    }
    else if (objc == 1) {
      res = SetupSet (interp, op, objv[0], ad);
    }
    else {
      Tcl_AppendResult (interp, "error: usage: \"", op, " ?value?\"", NULL);
      res = TCL_ERROR;
    }
  }

  return res;
}

/*
 * Setup an invocation by DII
 */

int
Combat::ObjectRequest::SetupDii (Tcl_Interp * interp, Context * c,
				 Tcl_Obj *CONST spec,
				 int objc, Tcl_Obj *CONST objv[])
{
  Tcl_Obj *rtypeobj, *opnameobj, *paramsobj, *exceptionsobj;
  int res, len, i, numparams, numexcepts;
  const char * opname;

  assert (obj);
  ctx = c;

  if (obj->pseudo) {
    Tcl_AppendResult (interp, "error: cannot use dii on pseudo objects",
		      NULL);
    return TCL_ERROR;
  }

  if (Tcl_ListObjLength (NULL, spec, &len) != TCL_OK || len < 3 ||
      Tcl_ListObjIndex (NULL, spec, 0, &rtypeobj) != TCL_OK ||
      Tcl_ListObjIndex (NULL, spec, 1, &opnameobj) != TCL_OK ||
      (opname = Tcl_GetStringFromObj (opnameobj, NULL)) == NULL ||
      Tcl_ListObjIndex (NULL, spec, 2, &paramsobj) != TCL_OK ||
      Tcl_ListObjLength (NULL, paramsobj, &numparams) != TCL_OK) {
    Tcl_AppendResult (interp, "error: invalid dii spec",
		      NULL);
    return TCL_ERROR;
  }

  if (numparams != objc) {
    char tmp[64];
    sprintf (tmp, "%lu", numparams);
    Tcl_AppendResult (interp, "error: not enough parameters, ",
		      "expecting ", tmp, NULL);
    return TCL_ERROR;
  }

  if (len >= 4) {
    if (Tcl_ListObjIndex (NULL, spec, 3, &exceptionsobj) != TCL_OK ||
	Tcl_ListObjLength (NULL, exceptionsobj, &numexcepts) != TCL_OK) {
      Tcl_AppendResult (interp, "error: invalid dii spec",
			NULL);
      return TCL_ERROR;
    }
  }
  else {
    exceptionsobj = NULL;
    numexcepts = 0;
  }

  if (len >= 5) {
    Tcl_Obj * owobj;
    Tcl_ListObjIndex (NULL, spec, 4, &owobj);
    const char * owstr = Tcl_GetStringFromObj (owobj, NULL);
    if (strcmp (owstr, "OP_ONEWAY") == 0 ||
	strcmp (owstr, "oneway") == 0) {
      is_oneway = true;
    }
  }

  /*
   * Build request
   */

  req = obj->obj->_request (opname);

  /*
   * Set result type
   */

  rtype = Combat::GetTypeCodeFromObj (interp, rtypeobj);

  if (CORBA::is_nil (rtype)) {
    Tcl_AppendResult (interp, "\nerror: invalid return typecode in dii spec",
		      NULL);
    return TCL_ERROR;
  }

  req->set_return_type (rtype.in());

  /*
   * User Exceptions
   */

  for (i=0; i<numexcepts; i++) {
    Tcl_Obj * theex;
    if (Tcl_ListObjIndex (NULL, exceptionsobj, 0, &theex) != TCL_OK) {
      Tcl_AppendResult (interp, "error: invalid dii spec",
			NULL);
      return TCL_ERROR;
    }

    CORBA::TypeCode_var extc = Combat::GetTypeCodeFromObj (interp, theex);

    if (CORBA::is_nil (extc)) {
      Tcl_AppendResult (interp, "\nerror: invalid exception typecode ",
			"in dii spec", NULL);
      return TCL_ERROR;
    }

    req->exceptions()->add (extc.in());
  }

  /*
   * Parameters
   */

  pds = new CORBA::ParDescriptionSeq;
  pds->length (numparams);

  for (i=0; i<numparams; i++) {
    Tcl_Obj *paramspec, *paramdirobj, *paramtypeobj;
    const char *paramdirstr;
    CORBA::TypeCode_var ptc;
    CORBA::Any * any;
    CORBA::Flags mode;
    int parspeclen;

    if (Tcl_ListObjIndex (NULL, paramsobj, i, &paramspec) != TCL_OK ||
	Tcl_ListObjLength (NULL, paramspec, &parspeclen) != TCL_OK ||
	parspeclen != 2 ||
	Tcl_ListObjIndex (NULL, paramspec, 0, &paramdirobj) != TCL_OK ||
	(paramdirstr = Tcl_GetStringFromObj (paramdirobj, NULL)) == NULL ||
	Tcl_ListObjIndex (NULL, paramspec, 1, &paramtypeobj) != TCL_OK) {
      char tmp[64];
      sprintf (tmp, "%d", i);
      Tcl_AppendResult (interp, "error: invalid param spec in dii spec ",
			"at index ", tmp, NULL);
      return TCL_ERROR;
    }

    ptc = Combat::GetTypeCodeFromObj (interp, paramtypeobj);

    if (CORBA::is_nil (ptc)) {
      char tmp[64];
      sprintf (tmp, "%d", i);
      Tcl_AppendResult (interp, "\nerror: invalid param typecode ",
			"in dii spec at index ", tmp, NULL);
      return TCL_ERROR;
    }


    if (strcmp (paramdirstr, "PARAM_IN") == 0 ||
	strcmp (paramdirstr, "in") == 0) {
      (*pds)[i].mode = CORBA::PARAM_IN;
      (*pds)[i].type = CORBA::TypeCode::_duplicate (ptc);
      any = Combat::GetAnyFromObj (interp, ctx, objv[i], ptc.in());
      mode = CORBA::ARG_IN;
    }
    else if (strcmp (paramdirstr, "PARAM_OUT") == 0 ||
	     strcmp (paramdirstr, "out") == 0) {
      (*pds)[i].mode = CORBA::PARAM_OUT;
      (*pds)[i].type = CORBA::TypeCode::_duplicate (ptc);
      any = new CORBA::Any (ptc.in(), (void *) NULL);
      mode = CORBA::ARG_OUT;
    }
    else if (strcmp (paramdirstr, "PARAM_INOUT") == 0 ||
	     strcmp (paramdirstr, "inout") == 0) {
      Tcl_Obj * data;

      if ((data = Tcl_ObjGetVar2 (interp, objv[i], NULL,
				  TCL_PARSE_PART1)) == NULL) {
	Tcl_AppendResult (interp, "can't read \"",
			  Tcl_GetStringFromObj (objv[i], NULL),
			  "\": no such variable", NULL);
	any = NULL;
	break;
      }

      (*pds)[i].mode = CORBA::PARAM_INOUT;
      (*pds)[i].type = CORBA::TypeCode::_duplicate (ptc);

      any = Combat::GetAnyFromObj (interp, ctx, data, ptc.in());
      mode = CORBA::ARG_INOUT;
    }
    else {
      char tmp[64];
      sprintf (tmp, "%d", i);
      Tcl_AppendResult (interp, "error: invalid param dir in dii spec ",
			"at index ", tmp, NULL);
    }

    if (!any) {
      char tmp[64];
      sprintf (tmp, "%d", i);
      Tcl_AppendResult (interp, "\n  while packing parameter ",
			tmp, " of dii operation \"", opname, "\"", NULL);
      return TCL_ERROR;
    }

    req->arguments()->add_value_consume (CORBA::string_dup (""), any, mode);
  }

  /*
   * Save information about out/inout parameters
   */

  params = new Tcl_Obj * [numparams];

  for (i=0; i < numparams; i++) {
    Tcl_IncrRefCount (objv[i]);
    params[i] = objv[i];
  }

  return TCL_OK;
}

/*
 * Send / Query invocation
 */

int
Combat::ObjectRequest::Invoke (Tcl_Interp *)
{
  assert (is_builtin || !CORBA::is_nil (req));

  if (is_builtin) {
  }
  else if (params && is_oneway) {
    req->send_oneway ();
  }
  else {
    req->send_deferred ();
  }

  return TCL_OK;
}

bool
Combat::ObjectRequest::PollResult (void)
{
  assert (is_builtin || !CORBA::is_nil (req));

  bool res;

  if (is_builtin) {
    res = true;
  }
  else if (params && is_oneway) {
    res = true;
  }
  else if (is_finished) {
    res = true;
  }
  else {
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      if ((res = is_finished = req->poll_response ())) {
	req->get_response ();
      }
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      req_except = Combat::DecodeException (NULL, ctx, &ex);
      Tcl_IncrRefCount (req_except);
      is_finished = true;
    }
#endif
  }

  return res;
}

/*
 * Handle completed request
 */

int
Combat::ObjectRequest::GetResult (Tcl_Interp * interp)
{
  assert (is_builtin || !CORBA::is_nil (req));

  /*
   * Handle builtin result
   */

  if (is_builtin) {
    Tcl_SetObjResult (interp, builtin_result);
    return TCL_OK;
  }

  /*
   * If request is oneway, there's no response
   */

  if (is_oneway) {
    return TCL_OK;
  }

  /*
   * Make sure result is available
   */

  if (!is_finished) {
#ifdef HAVE_EXCEPTIONS
    try {
#endif
      req->get_response ();
#ifdef HAVE_EXCEPTIONS
    } catch (CORBA::Exception &ex) {
      req_except = Combat::DecodeException (NULL, ctx, &ex);
      Tcl_IncrRefCount (req_except);
    }
#endif
  }

  if (req_except) {
    Tcl_SetObjResult (interp, req_except);
    return TCL_ERROR;
  }

  if (req->env()->exception()) {
    Tcl_Obj * exobj = Combat::DecodeException (interp, ctx,
					       req->env()->exception());
    Tcl_SetObjResult (interp, exobj);
    return TCL_ERROR;
  }

  /*
   * Handle out/inout arguments, if necessary
   */

  if (params) {
    for (CORBA::ULong i=0; i < pds->length(); i++) {
      Tcl_Obj * data;

      switch ((*pds)[i].mode) {
      case CORBA::PARAM_OUT:
      case CORBA::PARAM_INOUT:
	data = Combat::NewAnyObj (interp, ctx,
				   *req->arguments()->item(i)->value());

	if (Tcl_ObjSetVar2 (interp, params[i], NULL,
			    data, TCL_PARSE_PART1) == NULL) {
	  Tcl_AppendResult (interp, "can't set variable \"",
			    Tcl_GetStringFromObj (params[i], NULL),
			    "\n  while extracting parameter ",
			    (*pds)[i].name.in(),
			    "\"", NULL);
	  Tcl_DecrRefCount (data);
	  return TCL_ERROR;
	}
	break;

      case CORBA::PARAM_IN:
	break;
      }
    }
  }

  /*
   * Extract result, if necessary
   */


  if (!CORBA::is_nil (rtype)) {
    Tcl_Obj * res = Combat::NewAnyObj (interp, ctx, *req->result()->value());
    Tcl_SetObjResult (interp, res);
  }
  else {
    Tcl_SetObjResult (interp, Tcl_NewObj ());
  }

  return TCL_OK;
}
