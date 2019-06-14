// -*- c++ -*-

/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
#ifndef __COMBAT_MICO_BINDER_H__
#define __COMBAT_MICO_BINDER_H__

#include "combat.h"

COMBAT_NAMESPACE MicoBinder {

class BindRequest : virtual public Combat::Request
{
public:
  BindRequest ();
  ~BindRequest ();

  int  Setup          (Tcl_Interp *, Combat::Context *,
		       int objc, Tcl_Obj *CONST []);
  int  Invoke         (Tcl_Interp *);
  int  GetResult      (Tcl_Interp *);
  bool PollResult     (void);

private:
  Tcl_Obj * tag;
  Tcl_Obj * address;
  Tcl_Obj * repoid;
  CORBA::Object_var res;
  Combat::Context * ctx;
};

}; // end of namespace MicoBinder

#endif
