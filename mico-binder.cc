/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
#include "combat.h"
#include "mico-binder.h"
#include <assert.h>

char * combat_mico_binder_id = "$Id$";

MicoBinder::BindRequest::BindRequest ()
{
  tag = address = repoid = NULL;
}

MicoBinder::BindRequest::~BindRequest ()
{
  if (tag) {
    Tcl_DecrRefCount (tag);
  }
  if (address) {
    Tcl_DecrRefCount (address);
  }
  if (repoid) {
    Tcl_DecrRefCount (repoid);
  }
}

int
MicoBinder::BindRequest::Setup (Tcl_Interp * interp, Combat::Context * c,
				int objc, Tcl_Obj *CONST objv[])
{
  ctx = c;

  int i;
  for (i=0; i<objc; i++) {
    if (strcmp (Tcl_GetStringFromObj (objv[i], NULL), "-addr") == 0 &&
	!address) {
      if (++i == objc) {
	break;
      }
      address = objv[i];
      Tcl_IncrRefCount (address);
    }
    else if (!repoid) {
      repoid = objv[i];
      Tcl_IncrRefCount (repoid);
    }
    else if (!tag) {
      tag = objv[i];
      Tcl_IncrRefCount (tag);
    }
    else {
      break;
    }
  }

  if (i != objc || !repoid) {
    Tcl_AppendResult (interp, "wrong # args: should be \"",
		      "mico::bind ?-addr addr? repoid ?id?\"", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
MicoBinder::BindRequest::Invoke (Tcl_Interp * interp)
{
  assert (repoid);

  CORBA::ORB::ObjectTag_var ot;
  const char * id, * ad = NULL;

  id = Tcl_GetStringFromObj (repoid, NULL);

  if (address) {
    ad = Tcl_GetStringFromObj (address, NULL);
  }

  if (tag) {
    int len;
    const char * strtag = Tcl_GetStringFromObj (tag, &len);
    ot = new CORBA::ORB::ObjectTag (len, len, (CORBA::Octet *) strtag);
  }
  else {
    ot = new CORBA::ORB::ObjectTag;
  }

  res = Combat::GlobalData->orb->bind (id, ot.in(), ad);
  return TCL_OK;
}

int
MicoBinder::BindRequest::GetResult (Tcl_Interp * interp)
{
  if (CORBA::is_nil (res)) {
    Tcl_AppendResult (interp, "error: bind to \"",
		      Tcl_GetStringFromObj (repoid, NULL),
		      "\"", NULL);
    if (tag) {
      Tcl_AppendResult (interp, " using tag \"",
			Tcl_GetStringFromObj (tag, NULL),
			"\"", NULL);
    }
    if (address) {
      Tcl_AppendResult (interp, " at address \"",
			Tcl_GetStringFromObj (address, NULL),
			"\"", NULL);
    }
    Tcl_AppendResult (interp, " failed", NULL);
    return TCL_ERROR;
  }

  CORBA::Object_ptr dup = CORBA::Object::_duplicate (res.in());
  Tcl_Obj * ores;

  if ((ores = Combat::InstantiateObj (interp, ctx, dup)) == NULL) {
    CORBA::release (dup);
    return TCL_ERROR;
  }

  Tcl_SetObjResult (interp, ores);
  return TCL_OK;
}

bool
MicoBinder::BindRequest::PollResult ()
{
  return true;
}
