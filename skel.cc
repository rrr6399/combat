/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
#if defined(COMBAT_NO_SERVER_SIDE)
#error "skel.cc not needed when compiling without server-side support only!"
#endif

/*
 * ----------------------------------------------------------------------
 * Tcl Skeletons
 * ----------------------------------------------------------------------
 */

#include "combat.h"
#include <assert.h>

char * combat_skel_id = "$Id: skel.cc,v 1.30 2003/04/20 16:09:35 fp Exp $";

/*
 * ----------------------------------------------------------------------
 *
 * Bootstrapping code for PortableServer module
 *
 * ----------------------------------------------------------------------
 */

#ifdef HAVE_NAMESPACE
namespace Combat {
const char * PortableServerCode
#else
const char * Combat::PortableServerCode
#endif
= "\
#\n\
# PortableServer module for Combat\n\
#\n\
\n\
catch {namespace import itcl::*}\n\
\n\
namespace eval PortableServer {\n\
    class ServantBase {\n\
	constructor {} {\n\
	    combat::servant NewServant $this [_Interface]\n\
	}\n\
	destructor {\n\
	    combat::servant DeleteServant $this\n\
	}\n\
	public method _this {} {\n\
	    return [combat::servant _this $this]\n\
	}\n\
	public method _Interface {} {\n\
	    error \"_interface not overloaded\"\n\
	}\n\
    }\n\
}\n\
";
#ifdef HAVE_NAMESPACE
};
#endif

/*
 * ----------------------------------------------------------------------
 *
 * PortableServer::ServantBase
 *
 * ----------------------------------------------------------------------
 */

Combat::Servant::Servant (Tcl_Interp * _i,
			  Tcl_Obj * _o,
			  Context * _c)
  : interp (_i), obj (_o), ctx (_c)
{
  assert (interp);
  assert (obj);
  assert (ctx);
  Tcl_IncrRefCount (obj);
}

Combat::Servant::~Servant ()
{
  Tcl_DecrRefCount (obj);
}

/*
 * ----------------------------------------------------------------------
 *
 * PortableServer::DynamicServant
 *
 * ----------------------------------------------------------------------
 */

Combat::DynamicServant::DynamicServant (Tcl_Interp * _i, Tcl_Obj * _o,
					Context * _c,
					CORBA::InterfaceDef_ptr _if)
  : Combat::Servant (_i, _o, _c)
{
  iface = Combat::GlobalData->icache.insert (_if);
  assert (iface != NULL);
}

Combat::DynamicServant::~DynamicServant ()
{
  Combat::GlobalData->icache.remove (iface->id());
}

CORBA::Object_ptr
Combat::DynamicServant::_this ()
{
  return PortableServer::DynamicImplementation::_this ();
}

void
Combat::DynamicServant::dispatch_invoke (const char * op,
					 CORBA::ServerRequest_ptr svr,
					 CORBA::OperationDescription * od)
{
  CORBA::ULong inouts=0, outs=0;
  Tcl_Obj *name, *value;
  char tmp[256];

  CORBA::NVList_ptr args;
  Combat::GlobalData->orb->create_list (0, args);

  for (CORBA::ULong i0=0; i0 < od->parameters.length(); i0++) {
    CORBA::Flags mode;

    switch (od->parameters[i0].mode) {
    case CORBA::PARAM_IN:    mode = CORBA::ARG_IN;    break;
    case CORBA::PARAM_OUT:   mode = CORBA::ARG_OUT;   outs++; break;
    case CORBA::PARAM_INOUT: mode = CORBA::ARG_INOUT; inouts++; break;
    default:
      assert (0);
    }

    CORBA::Any * any = new CORBA::Any (od->parameters[i0].type, (void *) NULL);
    args->add_value_consume (CORBA::string_dup (""), any, mode);
  }

  svr->arguments (args);

  /*
   * We must pass variable names for inout and out parameters.
   */

  for (CORBA::ULong i1=0; i1 < od->parameters.length(); i1++) {
    if (od->parameters[i1].mode == CORBA::PARAM_INOUT) {
      sprintf (tmp, "_combat_arg_%02lu", i1);
      name  = Tcl_NewStringObj (tmp, -1);
      value = Combat::NewAnyObj (interp, ctx, *args->item(i1)->value());
      Tcl_ObjSetVar2 (interp, name, NULL, value, 0);
      Tcl_DecrRefCount (name);
    }
  }

  /*
   * Build command line
   */

  Tcl_Obj ** c = new Tcl_Obj *[od->parameters.length()+2];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ((char *) op, -1);

  for (CORBA::ULong i3=0; i3 < od->parameters.length(); i3++) {
    switch (od->parameters[i3].mode) {
    case CORBA::PARAM_IN:
      c[i3+2] = Combat::NewAnyObj (interp, ctx, *args->item(i3)->value());
      break;
    case CORBA::PARAM_INOUT:
    case CORBA::PARAM_OUT:
      sprintf (tmp, "_combat_arg_%02lu", i3);
      c[i3+2] = Tcl_NewStringObj (tmp, -1);
      break;
    }
  }

  Tcl_Obj * com = Tcl_NewListObj (od->parameters.length()+2, c);
  Tcl_IncrRefCount (com);

  /*
   * Execute
   */

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  int res = Tcl_EvalObj (interp, com);
#else
  int res = Tcl_EvalObjv (interp, od->parameters.length()+2, c, 0);
#endif

  Tcl_Obj * ores = Tcl_GetObjResult (interp);
  Tcl_IncrRefCount (ores);
  Tcl_DecrRefCount (com);
  delete [] c;

  /*
   * In case of an error, there are two possibilities. First, the operation
   * could have thrown an exception. Second, the operation may have had an
   * error.
   */

  if (res != TCL_OK) {
    CORBA::Any * ex = Combat::EncodeException (interp, ctx, ores);

    if (!ex) {
      Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
      Tcl_AddErrorInfo (interp, (char *) op);
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);

      ex = new CORBA::Any;
      *ex <<= CORBA::UNKNOWN (0, CORBA::COMPLETED_MAYBE);
    }

    Tcl_DecrRefCount (ores);
    svr->set_exception (*ex);
    delete ex;
    return;
  }

  /*
   * Get Return value if return type != void
   */

  if (od->result->kind() != CORBA::tk_void) {
    CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, ores,
					       od->result.in());

    if (!any) {
      Tcl_AddErrorInfo (interp, "\n  while packing return value");
      Tcl_AddErrorInfo (interp, "\n  after invoking operation \"");
      Tcl_AddErrorInfo (interp, (char *) op);
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);

      CORBA::Any ex;
      ex <<= CORBA::MARSHAL (0, CORBA::COMPLETED_YES);
      svr->set_exception (ex);
      Tcl_DecrRefCount (ores);
      return;
    }

    svr->set_result (*any);
    delete any;
  }

  Tcl_DecrRefCount (ores);

  /*
   * Retrieve inout and out parameters
   */

  for (CORBA::ULong i4=0; i4 < od->parameters.length(); i4++) {
    if (od->parameters[i4].mode == CORBA::PARAM_INOUT ||
	od->parameters[i4].mode == CORBA::PARAM_OUT) {

      sprintf (tmp, "_combat_arg_%02lu", i4);
      name  = Tcl_NewStringObj (tmp, -1);
      value = Tcl_ObjGetVar2 (interp, name, NULL, 0);
      Tcl_DecrRefCount (name);
      
      if (value == NULL) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: variable for ", NULL);
	if (od->parameters[i4].mode == CORBA::PARAM_INOUT) {
	  Tcl_AppendResult (interp, "inout", NULL);
	}
	else {
	  Tcl_AppendResult (interp, "out", NULL);
	}
	Tcl_AppendResult (interp, "parameter \"",
			  (char *) od->parameters[i4].name.in(),
			  "\" not set after invoking \"", op,
			  "\" on object \"",
			  Tcl_GetStringFromObj (obj, NULL),
			  "\"", NULL);
	Tcl_BackgroundError (interp);

	CORBA::Any ex;
	ex <<= CORBA::INTERNAL (0, CORBA::COMPLETED_YES);
	svr->set_exception (ex);
	return;
      }
      
      CORBA::Any * par = Combat::GetAnyFromObj (interp, ctx, value,
						od->parameters[i4].type);
      Tcl_UnsetVar (interp, tmp, 0);
      
      if (!par) {
	Tcl_AddErrorInfo (interp, "\n  while packing parameter \"");
	Tcl_AddErrorInfo (interp, (char *) od->parameters[i4].name.in());
	Tcl_AddErrorInfo (interp, "\n  after invoking operation \"");
	Tcl_AddErrorInfo (interp, (char *) op);
	Tcl_AddErrorInfo (interp, "\" for object \"");
	Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
	Tcl_AddErrorInfo (interp, "\"");
	Tcl_BackgroundError (interp);

	CORBA::Any ex;
	ex <<= CORBA::MARSHAL (0, CORBA::COMPLETED_YES);
	svr->set_exception (ex);
	return;
      }
      
      *args->item(i4)->value() = *par;
      delete par;
    }
  }
}

void
Combat::DynamicServant::dispatch_attr_get (const char * attr,
					   CORBA::ServerRequest_ptr svr,
					   CORBA::AttributeDescription * ad)
{
  CORBA::NVList_ptr args;
  Combat::GlobalData->orb->create_list (0, args);
  svr->arguments (args);

  /*
   * obj cget -attr
   */

  Tcl_Obj *com, *c[3];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("cget", 4);
  c[2] = Tcl_NewStringObj ("-", 1);
  Tcl_AppendStringsToObj (c[2], attr, NULL);
  com  = Tcl_NewListObj (3, c);
  Tcl_IncrRefCount (com);

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  int res = Tcl_EvalObj (interp, com);
#else
  int res = Tcl_EvalObjv (interp, 3, c, 0);
#endif

  Tcl_Obj * ores = Tcl_GetObjResult (interp);
  Tcl_IncrRefCount (ores);

  if (res != TCL_OK) {
    CORBA::Any * ex = Combat::EncodeException (interp, ctx, ores);

    if (!ex) {
      Tcl_AddErrorInfo (interp, "\n  while getting attribute \"");
      Tcl_AddErrorInfo (interp, (char *) attr);
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);

      ex = new CORBA::Any;
      *ex <<= CORBA::UNKNOWN (0, CORBA::COMPLETED_NO);
    }

    svr->set_exception (*ex);
    delete ex;
  }
  else {
    CORBA::Any * any = Combat::GetAnyFromObj (interp, ctx, ores,
					       ad->type.in());

    if (!any) {
      Tcl_AddErrorInfo (interp, "\n  while packing return value");
      Tcl_AddErrorInfo (interp, "\n  after getting attribute \"");
      Tcl_AddErrorInfo (interp, (char *) attr);
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);

      CORBA::Any uex;
      uex <<= CORBA::MARSHAL (0, CORBA::COMPLETED_NO);
      svr->set_exception (uex);
    }
    else {
      svr->set_result (*any);
      delete any;
    }
  }

  Tcl_DecrRefCount (ores);
  Tcl_DecrRefCount (com);
}

void
Combat::DynamicServant::dispatch_attr_set (const char * attr,
					   CORBA::ServerRequest_ptr svr,
					   CORBA::AttributeDescription * ad)
{
  CORBA::NVList_ptr args;
  Combat::GlobalData->orb->create_list (0, args);
  CORBA::Any * any = new CORBA::Any (ad->type.in(), (void *) NULL);
  args->add_value_consume (CORBA::string_dup (""), any, CORBA::ARG_IN);
  svr->arguments (args);

  /*
   * obj configure -attr value
   */

  Tcl_Obj *com, *c[4];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("configure", 9);
  c[2] = Tcl_NewStringObj ("-", 1);
  Tcl_AppendStringsToObj (c[2], attr, NULL);
  c[3] = Combat::NewAnyObj (interp, ctx, *args->item(0)->value());
  com  = Tcl_NewListObj (4, c);
  Tcl_IncrRefCount (com);

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  int res = Tcl_EvalObj (interp, com);
#else
  int res = Tcl_EvalObjv (interp, 4, c, 0);
#endif

  Tcl_Obj * ores = Tcl_GetObjResult (interp);
  Tcl_IncrRefCount (ores);

  if (res != TCL_OK) {
    CORBA::Any * ex = Combat::EncodeException (interp, ctx, ores);

    if (!ex) {
      Tcl_AddErrorInfo (interp, "\n  while setting attribute \"");
      Tcl_AddErrorInfo (interp, (char *) attr);
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);

      ex = new CORBA::Any;
      *ex <<= CORBA::UNKNOWN (0, CORBA::COMPLETED_MAYBE);
    }

    svr->set_exception (*ex);
    delete ex;
  }
  
  Tcl_DecrRefCount (ores);
  Tcl_DecrRefCount (com);
}

void
Combat::DynamicServant::invoke (CORBA::ServerRequest_ptr svr)
{
  /*
   * Operation or Attribute
   */

  const char * op = svr->operation ();
  bool isset;

  if (strncmp (op, "_set_", 5) == 0) {
    isset = true;
    op += 5;
  }
  else if (strncmp (op, "_get_", 5) == 0) {
    isset = false;
    op += 5;
  }

  CORBA::OperationDescription * od;
  CORBA::AttributeDescription * ad;

  if (iface->lookup (op, od, ad)) {
    assert (od != NULL || ad != NULL);
    if (od != NULL) {
      dispatch_invoke (op, svr, od);
    }
    else if (ad != NULL && isset) {
      dispatch_attr_set (op, svr, ad);
    }
    else {
      dispatch_attr_get (op, svr, ad);
    }
    return;
  }

  CORBA::Any ex;
  ex <<= CORBA::BAD_OPERATION (0, CORBA::COMPLETED_NO);
  svr->set_exception (ex);
}

CORBA::RepositoryId
Combat::DynamicServant::_primary_interface (const PortableServer::ObjectId &,
					    PortableServer::POA_ptr)
{
  return CORBA::string_dup (iface->id());
}

CORBA::Boolean
Combat::DynamicServant::_is_a (const char * repoid)
{
  CORBA::InterfaceDef_var ifd = iface->iface ();
  return ifd->is_a (repoid);
}

/*
 * ----------------------------------------------------------------------
 *
 * PortableServer::DynamicImplementation
 *
 * ----------------------------------------------------------------------
 */

Combat::DynamicImplementation::DynamicImplementation (Tcl_Interp * _i,
						      Tcl_Obj * _o,
						      Context * _c)
  : Combat::Servant (_i, _o, _c)
{
}

Combat::DynamicImplementation::~DynamicImplementation ()
{
}

CORBA::Object_ptr
Combat::DynamicImplementation::_this ()
{
  return PortableServer::DynamicImplementation::_this ();
}

void
Combat::DynamicImplementation::invoke (CORBA::ServerRequest_ptr svr)
{
  /*
   * Create a new ServerRequest pseudo object
   */

  Combat::ServerRequest * msvr = new Combat::ServerRequest (svr);
  Tcl_Obj * treq = Combat::InstantiateObj (interp, ctx, msvr);
  Tcl_IncrRefCount (treq);

  Tcl_Obj * com, * c[3];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("invoke", 6);
  c[2] = treq;
  com  = Tcl_NewListObj (3, c);
  Tcl_IncrRefCount (com);

  /*
   * Invoke invoke
   */

  int res = Tcl_EvalObj (interp, com);
  Tcl_Obj * ores = Tcl_GetObjResult (interp);
  Tcl_IncrRefCount (ores);
  Tcl_DecrRefCount (com);
  Tcl_DecrRefCount (treq);

  if (res != TCL_OK) {
    CORBA::Any * ex = Combat::EncodeException (interp, ctx, ores);

    if (!ex) {
      Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
      Tcl_AddErrorInfo (interp, (char *) svr->operation());
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);

      ex = new CORBA::Any;
      *ex <<= CORBA::UNKNOWN (0, CORBA::COMPLETED_MAYBE);
    }

    svr->set_exception (*ex);
    Tcl_DecrRefCount (ores);
    delete ex;
    return;
  }

  Tcl_DecrRefCount (ores);
}

CORBA::Boolean
Combat::DynamicImplementation::_is_a (const char * repoid)
  throw (CORBA::SystemException)
{
  int res;

  Tcl_Obj * com, * c[3];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("_is_a", 18);
  c[2] = Tcl_NewStringObj ((char *) repoid, -1);
  com  = Tcl_NewListObj (3, c);
  Tcl_IncrRefCount (com);

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "_is_a");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);
    res = 0;
  }
  else {
    Tcl_Obj * theres = Tcl_GetObjResult (interp);
    if (Tcl_GetBooleanFromObj (interp, theres, &res) != TCL_OK) {
      Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
      Tcl_AddErrorInfo (interp, "_is_a");
      Tcl_AddErrorInfo (interp, "\" for object \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_BackgroundError (interp);
      res = 0;
    }
  }

  Tcl_DecrRefCount (com);

  return res ? TRUE : FALSE;
}

CORBA::RepositoryId
Combat::DynamicImplementation::_primary_interface (const PortableServer::ObjectId & oid,
						   PortableServer::POA_ptr poa)
{
  char * result = NULL;

  Combat::POA * mpoa = new Combat::POA (poa);
  Tcl_Obj * poaobj = Combat::InstantiateObj (interp, ctx, mpoa);
  Tcl_IncrRefCount (poaobj);

  Tcl_Obj * com, * c[4];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("_primary_interface", 18);
  c[2] = Combat::ObjectId_to_Tcl_Obj (oid);
  c[3] = poaobj;
  com  = Tcl_NewListObj (4, c);
  Tcl_IncrRefCount (com);

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "_primary_interface");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);
  }
  else {
    Tcl_Obj * theres = Tcl_GetObjResult (interp);
    result = CORBA::string_dup (Tcl_GetStringFromObj (theres, NULL));
  }

  Tcl_DecrRefCount (poaobj);
  Tcl_DecrRefCount (com);

  if (result == NULL) {
    return CORBA::string_dup ("");
  }

  return result;
}

/*
 * ----------------------------------------------------------------------
 *
 * PortableServer::ServantManagers
 *
 * ----------------------------------------------------------------------
 */

Combat::ServantActivator::ServantActivator (Tcl_Interp * _i,
					    Tcl_Obj * _o,
					    Context * _c)
  : Combat::Servant (_i, _o, _c)
{
}

Combat::ServantActivator::~ServantActivator ()
{
}

CORBA::Object_ptr
Combat::ServantActivator::_this ()
{
  return POA_PortableServer::ServantActivator::_this ();
}

PortableServer::Servant
Combat::ServantActivator::incarnate (const PortableServer::ObjectId & oid,
				     PortableServer::POA_ptr poa)
  throw (PortableServer::ForwardRequest, CORBA::SystemException)
{
  Combat::POA * mpoa = new Combat::POA (poa);
  Tcl_Obj * poaobj = Combat::InstantiateObj (interp, ctx, mpoa);
  Tcl_IncrRefCount (poaobj);

  Tcl_Obj * com, * c[4];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("incarnate", 9);
  c[2] = Combat::ObjectId_to_Tcl_Obj (oid);
  c[3] = poaobj;
  com  = Tcl_NewListObj (4, c);
  Tcl_IncrRefCount (com);

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_Obj * ores = Tcl_GetObjResult (interp);
    Tcl_IncrRefCount (ores);

    CORBA::Any * ex = Combat::EncodeException (interp, ctx, ores);
    const PortableServer::ForwardRequest * fwr;

    Tcl_DecrRefCount (poaobj);
    Tcl_DecrRefCount (com);
    Tcl_DecrRefCount (ores);

    if (ex && (*ex >>= fwr)) {
      PortableServer::ForwardRequest fw2 (*fwr);
      delete ex;
      throw (fw2);
    }

    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "incarnate");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);

    delete ex;
    return NULL;
  }

  Tcl_Obj * theres = Tcl_GetObjResult (interp);
  Tcl_IncrRefCount (theres);
  Tcl_DecrRefCount (poaobj);
  Tcl_DecrRefCount (com);

  /*
   * servant should now be in our server map
   */

  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, theres);

  if (!serv) {
    Tcl_ResetResult (interp);
    Tcl_AppendResult (interp, "error: servant not initialized: \"",
		      Tcl_GetStringFromObj (theres, NULL), "\" ",
		      "after invoking incarnate", NULL);
    Tcl_BackgroundError (interp);
    Tcl_DecrRefCount (theres);
    return NULL;
  }

  Tcl_DecrRefCount (theres);
  return serv;
}

void
Combat::ServantActivator::etherealize (const PortableServer::ObjectId & oid,
				       PortableServer::POA_ptr poa,
				       PortableServer::Servant serv,
				       CORBA::Boolean cleanup_in_progress,
				       CORBA::Boolean wait_for_completion)
  throw (CORBA::SystemException)
{
  Combat::POA * mpoa = new Combat::POA (poa);
  Tcl_Obj * poaobj = Combat::InstantiateObj (interp, ctx, mpoa);
  Tcl_IncrRefCount (poaobj);

  /*
   * Search Servant in our map
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
    Tcl_ResetResult (interp);
    Tcl_AppendResult (interp, "error: oops: servant not found ",
		      "for etherealize", NULL);
    Tcl_BackgroundError (interp);
    return;
  }

  /*
   * Build command
   */

  Tcl_Obj * com, * c[7];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("etherealize", 11);
  c[2] = Combat::ObjectId_to_Tcl_Obj (oid);
  c[3] = poaobj;
  c[4] = thename;
  c[5] = Tcl_NewBooleanObj (cleanup_in_progress);
  c[6] = Tcl_NewBooleanObj (wait_for_completion);
  com  = Tcl_NewListObj (7, c);
  Tcl_IncrRefCount (com);

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "etherealize");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);
  }

  Tcl_DecrRefCount (poaobj);
  Tcl_DecrRefCount (com);
}

Combat::ServantLocator::ServantLocator (Tcl_Interp * _i,
					Tcl_Obj * _o,
					Context * _c)
  : Combat::Servant (_i, _o, _c)
{
}

Combat::ServantLocator::~ServantLocator ()
{
}

CORBA::Object_ptr
Combat::ServantLocator::_this ()
{
  return POA_PortableServer::ServantLocator::_this ();
}

PortableServer::Servant
Combat::ServantLocator::preinvoke (const PortableServer::ObjectId & oid,
				   PortableServer::POA_ptr poa,
				   const char * operation,
				   PortableServer::ServantLocator::Cookie &cookie)
    throw (PortableServer::ForwardRequest, CORBA::SystemException)
{
  Combat::POA * mpoa = new Combat::POA (poa);
  Tcl_Obj * poaobj = Combat::InstantiateObj (interp, ctx, mpoa);
  Tcl_IncrRefCount (poaobj);

  Tcl_Obj * com, * c[6];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("preinvoke", 9);
  c[2] = Combat::ObjectId_to_Tcl_Obj (oid);
  c[3] = poaobj;
  c[4] = Tcl_NewStringObj ((char *) operation, -1);
  c[5] = Tcl_NewStringObj ("_combat_Cookie", 14);
  com  = Tcl_NewListObj (6, c);
  Tcl_IncrRefCount (com);

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_Obj * ores = Tcl_GetObjResult (interp);
    Tcl_IncrRefCount (ores);

    CORBA::Any * ex = Combat::EncodeException (interp, ctx, ores);
    const PortableServer::ForwardRequest * fwr;

    Tcl_DecrRefCount (poaobj);
    Tcl_DecrRefCount (com);
    Tcl_DecrRefCount (ores);

    if (ex && (*ex >>= fwr)) {
      PortableServer::ForwardRequest fw2 (*fwr);
      delete ex;
      throw (fw2);
    }

    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "preinvoke");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);

    delete ex;
    return NULL;
  }

  Tcl_DecrRefCount (poaobj);
  Tcl_DecrRefCount (com);

  /*
   * servant should now be in our server map
   */

  Tcl_Obj * theres = Tcl_GetObjResult (interp);
  Combat::Servant * serv = Combat::FindServantByName (interp, ctx, theres);

  if (serv == NULL) {
    Tcl_ResetResult  (interp);
    Tcl_AppendResult (interp, "oops: servant \"",
		      Tcl_GetStringFromObj (theres, NULL), "\" not found",
		      NULL);
    Tcl_AppendResult (interp, "\n after invoking \"preinvoke\" ",
		      "on object \"", Tcl_GetStringFromObj (obj, NULL),
		      "\"", NULL);
    return NULL;
  }

  /*
   * Read Cookie "out" variable
   */

  com = Tcl_NewStringObj ("_combat_Cookie", 14);
  theres = Tcl_ObjGetVar2 (interp, com, NULL, TCL_LEAVE_ERR_MSG);
  Tcl_DecrRefCount (com);

  if (theres == NULL) {
    cookie = 0;
  }
  else {
    Tcl_IncrRefCount (theres);
    cookie = (void *) theres;
  }

  return serv;
}

void
Combat::ServantLocator::postinvoke (const PortableServer::ObjectId & oid,
				    PortableServer::POA_ptr poa,
				    const char * operation,
				    PortableServer::ServantLocator::Cookie cookie,
				    PortableServer::Servant serv)
    throw (CORBA::SystemException)
{
  Combat::POA * mpoa = new Combat::POA (poa);
  Tcl_Obj * poaobj = Combat::InstantiateObj (interp, ctx, mpoa);
  Tcl_IncrRefCount (poaobj);

  /*
   * Search Servant in our map
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
    Tcl_ResetResult (interp);
    Tcl_AppendResult (interp, "error: oops: servant not found ",
		      "for postinvoke", NULL);
    Tcl_BackgroundError (interp);
    return;
  }

  /*
   * Build command
   */

  Tcl_Obj * com, * c[7];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("postinvoke", 10);
  c[2] = Combat::ObjectId_to_Tcl_Obj (oid);
  c[3] = poaobj;
  c[4] = Tcl_NewStringObj ((char *) operation, -1);
  if (cookie) {
    c[5] = (Tcl_Obj *) cookie;
  }
  else {
    c[5] = Tcl_NewObj ();
  }
  c[6] = thename;
  com  = Tcl_NewListObj (7, c);
  Tcl_IncrRefCount (com);

  if (cookie) {
    Tcl_DecrRefCount ((Tcl_Obj *) cookie);
  }

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "postinvoke");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);
  }

  Tcl_DecrRefCount (poaobj);
  Tcl_DecrRefCount (com);
}

/*
 * ----------------------------------------------------------------------
 *
 * PortableServer::AdapterActivator
 *
 * ----------------------------------------------------------------------
 */

Combat::AdapterActivator::AdapterActivator (Tcl_Interp * _i,
					    Tcl_Obj * _o,
					    Context * _c)
  : Combat::Servant (_i, _o, _c)
{
}

Combat::AdapterActivator::~AdapterActivator ()
{
}

CORBA::Object_ptr
Combat::AdapterActivator::_this ()
{
  return POA_PortableServer::AdapterActivator::_this ();
}

CORBA::Boolean
Combat::AdapterActivator::unknown_adapter (PortableServer::POA_ptr parent,
					   const char * name)
  throw (CORBA::SystemException)
{
  Combat::POA * mpoa = new Combat::POA (parent);
  Tcl_Obj * poaobj = Combat::InstantiateObj (interp, ctx, mpoa);
  Tcl_IncrRefCount (poaobj);

  Tcl_Obj * com, * c[4];
  c[0] = obj;
  c[1] = Tcl_NewStringObj ("unknown_adapter", 15);
  c[2] = poaobj;
  c[3] = Tcl_NewStringObj ((char *) name, -1);
  com  = Tcl_NewListObj (4, c);
  Tcl_IncrRefCount (com);

  if (Tcl_EvalObj (interp, com) != TCL_OK) {
    Tcl_AddErrorInfo (interp, "\n  while invoking operation \"");
    Tcl_AddErrorInfo (interp, "unknown_adapter");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);
    return FALSE;
  }

  Tcl_DecrRefCount (poaobj);
  Tcl_DecrRefCount (com);

  /*
   * Get Boolean result
   */

  Tcl_Obj * theres = Tcl_GetObjResult (interp);
  int bval;

  if (Tcl_GetBooleanFromObj (NULL, theres, &bval) != TCL_OK) {
    Tcl_IncrRefCount (theres);
    Tcl_ResetResult  (interp);
    Tcl_AppendResult (interp, "error: \"",
		      Tcl_GetStringFromObj (theres, NULL),
		      "\" is not a boolean value ", NULL);
    Tcl_AddErrorInfo (interp, "\n  after invoking operation \"");
    Tcl_AddErrorInfo (interp, "unknown_adapter");
    Tcl_AddErrorInfo (interp, "\" for object \"");
    Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (obj, NULL));
    Tcl_AddErrorInfo (interp, "\"");
    Tcl_BackgroundError (interp);
    return FALSE;
  }

  return bval ? TRUE : FALSE;
}

