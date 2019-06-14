/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
#if defined(COMBAT_NO_COMBAT_IR)
#error "ir.cc not needed when compiling without combat::ir support!"
#endif

/*
 * ----------------------------------------------------------------------
 * Build Interface Repository from strings
 * ----------------------------------------------------------------------
 */

#include "combat.h"
#include <stdlib.h>
#include <assert.h>

char * combat_ir_id = "$Id: ir.cc,v 1.28 2004/12/14 01:40:04 fp Exp $";

/*
 * Helper Classes
 */

class CodeScanTcl {
public:
  CodeScanTcl (Tcl_Interp *, CORBA::Repository_ptr);
  int add     (Tcl_Obj *);
  int add     (Tcl_Obj *, CORBA::Container_ptr);

private:
  int addContained (Tcl_Obj *, CORBA::Container_ptr);
  int addContainer (Tcl_Obj *, CORBA::Container_ptr);
  int addModule    (Tcl_Obj *, CORBA::Container_ptr);
  int addConst     (Tcl_Obj *, CORBA::Container_ptr);
  int addStruct    (Tcl_Obj *, CORBA::Container_ptr);
  int addUnion     (Tcl_Obj *, CORBA::Container_ptr);
  int addException (Tcl_Obj *, CORBA::Container_ptr);
  int addEnum      (Tcl_Obj *, CORBA::Container_ptr);
  int addAlias     (Tcl_Obj *, CORBA::Container_ptr);
  int addAttribute (Tcl_Obj *, CORBA::Container_ptr);
  int addOperation (Tcl_Obj *, CORBA::Container_ptr);
  int addInterface (Tcl_Obj *, CORBA::Container_ptr);
  int addValue     (Tcl_Obj *, CORBA::Container_ptr);
  int addValueBox  (Tcl_Obj *, CORBA::Container_ptr);
  int addValueMember(Tcl_Obj *, CORBA::Container_ptr);
  int addNative    (Tcl_Obj *, CORBA::Container_ptr);

  int scanRepoid (Tcl_Obj *, char *&, char *&, char *&);
  CORBA::IDLType_ptr scanIDLTypeName (Tcl_Obj *);
  CORBA::Contained_ptr lookup_local (const char *, CORBA::Container_ptr);
  char * absolute_name (CORBA::IRObject_ptr);

  Tcl_Interp * interp;
  CORBA::Repository_var repo;
};

/*
 * Exported Wrapper functions
 */

int
Combat::IR_Add (Tcl_Interp * interp, Tcl_Obj * data,
		CORBA::Repository_ptr repo)
{
  CodeScanTcl cst (interp, repo);
  return cst.add (data);
}

int
Combat::IR_Add (Tcl_Interp * interp, Tcl_Obj * data,
		CORBA::Repository_ptr repo,
		const char * repoid)
{
  CORBA::Contained_var obj = repo->lookup_id (repoid);

  if (CORBA::is_nil (obj)) {
    obj = repo->lookup (repoid);
  }
  if (CORBA::is_nil (obj)) {
    Tcl_AppendResult (interp, "error: could not find \"", repoid,
		      "\" in repository", NULL);
    return TCL_ERROR;
  }

  CORBA::Container_var cv = CORBA::Container::_narrow (obj);

  if (CORBA::is_nil (cv)) {
    Tcl_AppendResult (interp, "error: could not find container \"",
		      repoid, "\" in repository", NULL);
    return TCL_ERROR;
  }
   
  CodeScanTcl cst (interp, repo);
  return cst.add (data, cv);
}

/*
 * ----------------------------------------------------------------------
 * Add to IR contents
 * ----------------------------------------------------------------------
 */

CodeScanTcl::CodeScanTcl (Tcl_Interp *_i, CORBA::Repository_ptr _r)
  : interp (_i)
{
  repo = CORBA::Repository::_duplicate (_r);
  assert (interp);
  assert (!CORBA::is_nil (repo));
}

int
CodeScanTcl::add (Tcl_Obj * data)
{
  return addContainer (data, repo);
}

int
CodeScanTcl::add (Tcl_Obj * data, CORBA::Container_ptr c)
{
  return addContained (data, c);
}

int
CodeScanTcl::scanRepoid (Tcl_Obj * data, char * &id,
			 char * &name, char * &version)
{
  Tcl_Obj * o1, * o2, *o3;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 3 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK) {
    Tcl_AppendResult (interp, "error: expected repoid ",
		      " (id/name/version) but got \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  id      = Tcl_GetStringFromObj (o1, NULL);
  name    = Tcl_GetStringFromObj (o2, NULL);
  version = Tcl_GetStringFromObj (o3, NULL);

  assert (id && name && version);
  assert (*id && *name && *version);

  return TCL_OK;
}

CORBA::IDLType_ptr
CodeScanTcl::scanIDLTypeName (Tcl_Obj * data)
{
  CORBA::IDLType_ptr res = CORBA::IDLType::_nil();
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len < 1) {
    Tcl_AppendResult (interp, "error: expected IDL type but got \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return CORBA::IDLType::_nil();
  }

  const char * str = Tcl_GetStringFromObj (data, NULL);

  if (strcmp (str, "void") == 0) {
    res = repo->get_primitive (CORBA::pk_void);
  }
  else if (strcmp (str, "short") == 0) {
    res = repo->get_primitive (CORBA::pk_short);
  }
  else if (strcmp (str, "long") == 0) {
    res = repo->get_primitive (CORBA::pk_long);
  }
  else if (strcmp (str, "unsigned short") == 0) {
    res = repo->get_primitive (CORBA::pk_ushort);
  }
  else if (strcmp (str, "unsigned long") == 0) {
    res = repo->get_primitive (CORBA::pk_ulong);
  }
  else if (strcmp (str, "float") == 0) {
    res = repo->get_primitive (CORBA::pk_float);
  }
  else if (strcmp (str, "double") == 0) {
    res = repo->get_primitive (CORBA::pk_double);
  }
  else if (strcmp (str, "boolean") == 0) {
    res = repo->get_primitive (CORBA::pk_boolean);
  }
  else if (strcmp (str, "char") == 0) {
    res = repo->get_primitive (CORBA::pk_char);
  }
  else if (strcmp (str, "octet") == 0) {
    res = repo->get_primitive (CORBA::pk_octet);
  }
  else if (strcmp (str, "any") == 0) {
    res = repo->get_primitive (CORBA::pk_any);
  }
  else if (strcmp (str, "TypeCode") == 0) {
    res = repo->get_primitive (CORBA::pk_TypeCode);
  }
  else if (strcmp (str, "Principal") == 0) {
    res = repo->get_primitive (CORBA::pk_Principal);
  }
  else if (strcmp (str, "string") == 0) {
    // bounded strings fail the above strcmp
    res = repo->get_primitive (CORBA::pk_string);
  }
  else if (strcmp (str, "Object") == 0) {
    res = repo->get_primitive (CORBA::pk_objref);
  }
  else if (strcmp (str, "long long") == 0) {
    res = repo->get_primitive (CORBA::pk_longlong);
  }
  else if (strcmp (str, "unsigned long long") == 0) {
    res = repo->get_primitive (CORBA::pk_ulonglong);
  }
  else if (strcmp (str, "long double") == 0) {
    res = repo->get_primitive (CORBA::pk_longdouble);
  }
  else if (strcmp (str, "wchar") == 0) {
    res = repo->get_primitive (CORBA::pk_wchar);
  }
  else if (strcmp (str, "wstring") == 0) {
    // bounded wstrings fail the above strcmp
    res = repo->get_primitive (CORBA::pk_wstring);
  }
  else if (strcmp (str, "value base") == 0) {
    res = repo->get_primitive (CORBA::pk_value_base);
  }
  else if (len == 1) {
    /*
     * An IDL type that we should be able too look up
     */

    CORBA::Contained_var cv = repo->lookup_id (str);

    res = CORBA::IDLType::_narrow (cv);

    if (CORBA::is_nil (res)) {
      Tcl_AppendResult (interp, "error: could not find type \"",
			str, "\" in repository", NULL);
      return CORBA::IDLType::_nil();
    }
  }
  else {
    Tcl_Obj *o1, *o2, *o3;
    CORBA::ULong bound;
    char *bend, *bstr;

    /*
     * A more complex unnamed type
     */

    if (Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK) {
      Tcl_AppendResult (interp, "error: expected IDL Type name, but got \"",
                        Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return CORBA::IDLType::_nil();
    }

    str = Tcl_GetStringFromObj (o1, NULL);

    if (strcmp (str, "string") == 0 && len == 2 &&
	Tcl_ListObjIndex (NULL, data, 1, &o2) == TCL_OK) {
      // bounded string

      bstr  = Tcl_GetStringFromObj (o2, NULL);
      bound = strtoul (bstr, &bend, 0);

      if (!*bend) {
	if (bound == 0) {
	  res = repo->get_primitive (CORBA::pk_string);
	}
	else {
	  res = repo->create_string (bound);
	}
      }
    }
    else if (strcmp (str, "wstring") == 0 && len == 2 &&
	     Tcl_ListObjIndex (NULL, data, 1, &o2) == TCL_OK) {
      // bounded wstring

      bstr  = Tcl_GetStringFromObj (o2, NULL);
      bound = strtoul (bstr, &bend, 0);

      if (!*bend) {
	if (bound == 0) {
	  res = repo->get_primitive (CORBA::pk_wstring);
	}
	else {
	  res = repo->create_wstring (bound);
	}
      }
    }
    else if (strcmp (str, "fixed") == 0 && len == 3 &&
	     Tcl_ListObjIndex (NULL, data, 1, &o2) == TCL_OK &&
	     Tcl_ListObjIndex (NULL, data, 2, &o3) == TCL_OK) {
      CORBA::UShort digits;
      CORBA::Short scale;

      CORBA::Any_var a1 = Combat::GetAnyFromObj (interp, NULL, o2,
						  CORBA::_tc_ushort);
      CORBA::Any_var a2 = Combat::GetAnyFromObj (interp, NULL, o3,
						  CORBA::_tc_short);

      if (a1 && a2 && (*a1 >>= digits) && (*a2 >>= scale)) {
	res = repo->create_fixed (digits, scale);
      }
    }
    else if (strcmp (str, "sequence") == 0 && (len == 2 || len == 3) &&
	     Tcl_ListObjIndex (NULL, data, 1, &o2) == TCL_OK) {
      CORBA::IDLType_var ct = scanIDLTypeName (o2);

      if (CORBA::is_nil (ct)) {
	Tcl_AppendResult (interp, "\n  while scanning sequence from \"",
			  Tcl_GetStringFromObj (data, NULL), "\"", NULL);
	return CORBA::IDLType::_nil();
      }

      if (len == 3) {
	if (Tcl_ListObjIndex (NULL, data, 2, &o3) == TCL_OK) {
	  bstr  = Tcl_GetStringFromObj (o3, NULL);
	  bound = strtoul (bstr, &bend, 0);
	  if (!*bend) {
	    res = repo->create_sequence (bound, ct);
	  }
	}
      }
      else {
	res = repo->create_sequence (0, ct);
      }
    }
    else if (strcmp (str, "array") == 0 && len == 3 &&
	     Tcl_ListObjIndex (NULL, data, 1, &o2) == TCL_OK &&
	     Tcl_ListObjIndex (NULL, data, 2, &o3) == TCL_OK) {
      CORBA::IDLType_var ct = scanIDLTypeName (o2);

      if (CORBA::is_nil (ct)) {
	Tcl_AppendResult (interp, "\n  while scanning array from \"",
			  Tcl_GetStringFromObj (data, NULL), "\"", NULL);
	return CORBA::IDLType::_nil();
      }

      bstr  = Tcl_GetStringFromObj (o3, NULL);
      bound = strtoul (bstr, &bend, 0);

      if (!*bend) {
	res = repo->create_array (bound, ct);
      }
    }
  }

  if (CORBA::is_nil (res)) {
    Tcl_AppendResult (interp, "error: expected IDL Type name, but got \"",
		      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
    return CORBA::IDLType::_nil();
  }

  return res;
}

CORBA::Contained_ptr
CodeScanTcl::lookup_local (const char * name, CORBA::Container_ptr c)
{
  CORBA::ContainedSeq_var cs = c->lookup_name (name, 1, CORBA::dk_all, TRUE);
  CORBA::ULong len = cs->length ();

  if (len == 0) {
    return CORBA::Contained::_nil ();
  }

  assert (len == 1);

  return CORBA::Contained::_duplicate (cs[(CORBA::ULong)0]);
}

char *
CodeScanTcl::absolute_name (CORBA::IRObject_ptr o)
{
  CORBA::Contained_var co = CORBA::Contained::_narrow (o);
  if (CORBA::is_nil (co)) {
    return CORBA::string_dup ("(repository)");
  }
  return co->absolute_name ();
}

int
CodeScanTcl::addContained (Tcl_Obj * data, CORBA::Container_ptr c)
{
  Tcl_Obj * item;
  const char * str;
  int res;

  if (Tcl_ListObjLength (NULL, data, &res) != TCL_OK) {
    return TCL_ERROR;
  }

  if (res == 0) {
    return TCL_OK;
  }

  if (Tcl_ListObjIndex (NULL, data, 0, &item) != TCL_OK) {
    Tcl_AppendResult (interp, "error: expected contained data, but got \"",
		      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
    return TCL_ERROR;
  }

  str = Tcl_GetStringFromObj (item, NULL);

  if (strcmp (str, "module") == 0) {
    res = addModule (data, c);
  }
  else if (strcmp (str, "const") == 0) {
    res = addConst (data, c);
  }
  else if (strcmp (str, "struct") == 0) {
    res = addStruct (data, c);
  }
  else if (strcmp (str, "exception") == 0) {
    res = addException (data, c);
  }
  else if (strcmp (str, "union") == 0) {
    res = addUnion (data, c);
  }
  else if (strcmp (str, "enum") == 0) {
    res = addEnum (data, c);
  }
  else if (strcmp (str, "typedef") == 0) {
    res = addAlias (data, c);
  }
  else if (strcmp (str, "attribute") == 0) {
    res = addAttribute (data, c);
  }
  else if (strcmp (str, "operation") == 0) {
    res = addOperation (data, c);
  }
  else if (strcmp (str, "interface") == 0 ||
	   strcmp (str, "localinterface") == 0 ||
	   strcmp (str, "abstractinterface") == 0) {
    res = addInterface (data, c);
  }
  else if (strcmp (str, "valuetype") == 0) {
    res = addValue (data, c);
  }
  else if (strcmp (str, "valuebox") == 0) {
    res = addValueBox (data, c);
  }
  else if (strcmp (str, "valuemember") == 0) {
    res = addValueMember (data, c);
  }
  else if (strcmp (str, "native") == 0) {
    res = addNative (data, c);
  }
  else {
    Tcl_AppendResult (interp, "error: unknown contained item \"",
		      str, "\"", NULL);
    res = TCL_ERROR;
  }

  if (res != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while adding \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  return res;
}

int
CodeScanTcl::addContainer (Tcl_Obj * data, CORBA::Container_ptr c)
{
  int i, len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK) {
    Tcl_AppendResult (interp, "error: no IR data: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (not a list)", NULL);
    return TCL_ERROR;
  }

  for (i=0; i<len; i++) {
    Tcl_Obj * item;
    int r;

    r = Tcl_ListObjIndex (NULL, data, i, &item);
    assert (r == TCL_OK);

    if (addContained (item, c) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  return TCL_OK;
}

int
CodeScanTcl::addModule (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3;
  const char *str;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 3 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "module") != 0) {
    Tcl_AppendResult (interp, "error: not a module: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like module data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning module from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::ModuleDef_var md;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    md = c->create_module (id, name, version);
    assert (!CORBA::is_nil (md));
  }
  else {
    md = CORBA::ModuleDef::_narrow (cv);
    if (CORBA::is_nil (md)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding module \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a module",
			NULL);
      return TCL_ERROR;
    }
    md->id (id);
    md->version (version);
  }

  if (addContainer (o3, md) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning module from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
CodeScanTcl::addConst (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3, *o4;
  const char * str;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 4 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "const") != 0) {
    Tcl_AppendResult (interp, "error: not a const: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like const data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning const from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::IDLType_var itv = scanIDLTypeName (o3);
  CORBA::TypeCode_var tc = itv->type();

  if (CORBA::is_nil (itv)) {
    Tcl_AppendResult (interp, "\n  while scanning const from \"",
                      Tcl_GetStringFromObj (data, NULL),
                      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::Any_var any = Combat::GetAnyFromObj (interp, NULL, o4, tc);

  if (!any) {
    Tcl_AppendResult (interp, "\n  while scanning const from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::ConstantDef_var cd;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    cd = c->create_constant (id, name, version, itv.in(), any.in());
    assert (!CORBA::is_nil (cd));
  }
  else {
    cd = CORBA::ConstantDef::_narrow (cv);
    if (CORBA::is_nil (cd)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding const \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a const",
			NULL);
      return TCL_ERROR;
    }
    cd->id (id);
    cd->version (version);
    cd->type_def (itv.in());
    cd->value (any.in());
  }

  return TCL_OK;
}

int
CodeScanTcl::addStruct (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3, *o4;
  const char * str;
  int len, nmems;

  /*
   * Can either be
   *   struct repoid members contained_decl
   * or
   *   struct repoid
   * for forward declarations
   */

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
      (len != 2 && len != 4) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (len == 4 && Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK) ||
      (len == 4 && Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK) ||
      (len == 4 && Tcl_ListObjLength (NULL, o3, &nmems) != TCL_OK) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "struct") != 0) {
    Tcl_AppendResult (interp, "error: not a struct: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like struct data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning struct from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Create structure first.
   */

  CORBA::Contained_var cv = lookup_local (name, c);
  CORBA::StructDef_var sd;
  CORBA::StructMemberSeq sms;
  sms.length (0);

  if (CORBA::is_nil (cv)) {
    sd = c->create_struct (id, name, version, sms);
  }
  else {
    sd = CORBA::StructDef::_narrow (cv);

    if (CORBA::is_nil (sd)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding struct \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a struct",
			NULL);
      return TCL_ERROR;
    }

    sd->id (id);
    sd->version (version);
  }

  /*
   * Forward reference only?
   */

  if (len == 2) {
    return TCL_OK;
  }

  /*
   * Scan contained members first; these are types that our members need.
   */

  if (addContainer (o4, sd) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning struct from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Scan Members
   */

  sms.length (nmems);

  for (int i=0; i<nmems; i++) {
    Tcl_Obj *mobj, *mno, *mto;
    const char *mname;
    int count, r;

    r = Tcl_ListObjIndex (NULL, o3, i, &mobj);
    assert (r == TCL_OK);

    if (Tcl_ListObjLength (NULL, mobj, &count) != TCL_OK || count != 2 ||
	Tcl_ListObjIndex (NULL, mobj, 0, &mno) != TCL_OK ||
	Tcl_ListObjIndex (NULL, mobj, 1, &mto) != TCL_OK) {
      Tcl_AppendResult (interp, "error: cannot read struct member from \"",
			Tcl_GetStringFromObj (mobj, NULL),
			"\" (should be <name> <type>)", NULL);
      Tcl_AppendResult (interp, "\n  while scanning struct from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    mname = Tcl_GetStringFromObj (mno, NULL);
    CORBA::IDLType_var itv = scanIDLTypeName (mto);

    if (CORBA::is_nil (itv)) {
      Tcl_AppendResult (interp, "\n  while scanning struct member from \"",
			Tcl_GetStringFromObj (mobj, NULL), "\"", NULL);
      Tcl_AppendResult (interp, "\n  while scanning struct from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    sms[i].name     = mname;
    sms[i].type     = CORBA::TypeCode::_duplicate (CORBA::_tc_void);
    sms[i].type_def = CORBA::IDLType::_duplicate (itv);
  }

  sd->members (sms);

  return TCL_OK;
}

int
CodeScanTcl::addUnion (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3, *o4, *o5;
  const char * str;
  int len, nmems;

  /*
   * Can either be
   *   union repoid disc_type members contained_decl
   * or
   *   union repoid
   * for forward declarations
   */

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
      (len != 2 && len != 5) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (len == 5 && Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK) ||
      (len == 5 && Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK) ||
      (len == 5 && Tcl_ListObjIndex (NULL, data, 4, &o5) != TCL_OK) ||
      (len == 5 && Tcl_ListObjLength (NULL, o4, &nmems) != TCL_OK) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "union") != 0) {
    Tcl_AppendResult (interp, "error: not a union: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like union data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning union from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Create union first.
   */

  CORBA::Contained_var cv = lookup_local (name, c);
  CORBA::IDLType_var dtype = repo->get_primitive (CORBA::pk_ulong);
  CORBA::UnionDef_var ud;
  CORBA::UnionMemberSeq ums;
  ums.length (0);

  if (CORBA::is_nil (cv)) {
    ud = c->create_union (id, name, version, dtype, ums);
    assert (!CORBA::is_nil (ud));
  }
  else {
    ud = CORBA::UnionDef::_narrow (cv);

    if (CORBA::is_nil (ud)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding union \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a union",
			NULL);
      return TCL_ERROR;
    }

    ud->id (id);
    ud->version (version);
  }

  /*
   * Forward reference only?
   */

  if (len == 2) {
    return TCL_OK;
  }

  /*
   * Get discriminator type
   */

  dtype = scanIDLTypeName (o3);

  if (CORBA::is_nil (dtype)) {
    Tcl_AppendResult (interp, "\n while scanning discriminator type ",
		      "for union from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::TypeCode_var dtc = dtype->type();
  ud->discriminator_type_def (dtype);

  /*
   * Scan contained members first; these are types that our members need.
   */

  if (addContainer (o5, ud) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning union from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Scan Members
   */

  ums.length (nmems);

  for (int i=0; i<nmems; i++) {
    Tcl_Obj *mobj, *mlab, *mnobj, *mtype;
    const char *mname, *lname;
    CORBA::Any label;
    int count, r;

    r = Tcl_ListObjIndex (NULL, o4, i, &mobj);
    assert (r == TCL_OK);

    if (Tcl_ListObjLength (NULL, mobj, &count) != TCL_OK || count != 3 ||
	Tcl_ListObjIndex (NULL, mobj, 0, &mlab) != TCL_OK ||
	Tcl_ListObjIndex (NULL, mobj, 1, &mnobj) != TCL_OK ||
	Tcl_ListObjIndex (NULL, mobj, 2, &mtype) != TCL_OK) {
      Tcl_AppendResult (interp, "error: cannot read union member from \"",
			Tcl_GetStringFromObj (mobj, NULL),
			"\" (should be <label> <name> <type>)", NULL);
      Tcl_AppendResult (interp, "\n  while scanning union from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    lname = Tcl_GetStringFromObj (mlab, NULL);

    if (strcmp (lname, "(default)") == 0) {
      DynamicAny::DynAny_var da = Combat::GlobalData->daf->create_dyn_any_from_type_code (CORBA::_tc_octet);
      da->insert_octet ('\0');
      CORBA::Any_var val = da->to_any();
      label = val.in();
    }
    else {
      CORBA::Any_var thelab = Combat::GetAnyFromObj (interp, NULL, mlab, dtc);
      if (!thelab) {
	Tcl_AppendResult (interp, "\n  while scanning union members from \"",
			  Tcl_GetStringFromObj (data, NULL), "\"", NULL);
	return TCL_ERROR;
      }
      label = *thelab;
    }

    mname = Tcl_GetStringFromObj (mnobj, NULL);
    CORBA::IDLType_var mtv = scanIDLTypeName (mtype);

    if (CORBA::is_nil (mtv)) {
      Tcl_AppendResult (interp, "\n  while scanning union members from \"",
			Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return TCL_ERROR;
    }

    ums[i].name     = mname;
    ums[i].label    = label;
    ums[i].type     = CORBA::TypeCode::_duplicate (CORBA::_tc_void);
    ums[i].type_def = CORBA::IDLType::_duplicate (mtv);
  }

  ud->members (ums);

  return TCL_OK;
}

int
CodeScanTcl::addException (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3, *o4;
  const char * str;
  int len, nmems;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 4 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK ||
      Tcl_ListObjLength (NULL, o3, &nmems) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "exception") != 0) {
    Tcl_AppendResult (interp, "error: not an exception: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like exception data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning exception from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Create exception and scan contained members first; these are
   * IDLTypes that our members need.
   */

  CORBA::StructMemberSeq sms;
  sms.length (0);

  CORBA::ExceptionDef_var ed;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    ed = c->create_exception (id, name, version, sms);
    assert (!CORBA::is_nil (ed));
  }
  else {
    ed = CORBA::ExceptionDef::_narrow (cv);
    if (CORBA::is_nil (ed)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding exception \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not an exception",
			NULL);
      return TCL_ERROR;
    }
    ed->id (id);
    ed->version (version);
  }

  if (addContainer (o4, ed) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning exception from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Scan Members
   */

  sms.length (nmems);

  for (int i=0; i<nmems; i++) {
    Tcl_Obj *mobj, *mno, *mto;
    const char *mname;
    int count, r;

    r = Tcl_ListObjIndex (NULL, o3, i, &mobj);
    assert (r == TCL_OK);

    if (Tcl_ListObjLength (NULL, mobj, &count) != TCL_OK || count != 2 ||
	Tcl_ListObjIndex (NULL, mobj, 0, &mno) != TCL_OK ||
	Tcl_ListObjIndex (NULL, mobj, 1, &mto) != TCL_OK) {
      Tcl_AppendResult (interp, "error: cannot read exception member from \"",
			Tcl_GetStringFromObj (mobj, NULL),
			"\" (should be <name> <type>)", NULL);
      Tcl_AppendResult (interp, "\n  while scanning exception from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    mname = Tcl_GetStringFromObj (mno, NULL);
    CORBA::IDLType_var itv = scanIDLTypeName (mto);

    if (CORBA::is_nil (itv)) {
      Tcl_AppendResult (interp, "\n  while scanning exception member from \"",
			Tcl_GetStringFromObj (mobj, NULL), "\"", NULL);
      Tcl_AppendResult (interp, "\n  while scanning exception from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    sms[i].name     = mname;
    sms[i].type     = CORBA::TypeCode::_duplicate (CORBA::_tc_void);
    sms[i].type_def = CORBA::IDLType::_duplicate (itv);
  }

  ed->members (sms);

  return TCL_OK;
}

int
CodeScanTcl::addEnum (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3;
  const char *str;
  int len, nelms;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 3 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjLength (NULL, o3, &nelms) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "enum") != 0) {
    Tcl_AppendResult (interp, "error: not an enum: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like enum data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning enum from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::EnumMemberSeq ems (nelms);
  ems.length (nelms);

  for (int i=0; i<nelms; i++) {
    Tcl_Obj * elm;
    int r;

    r = Tcl_ListObjIndex (NULL, o3, i, &elm);
    assert (r == TCL_OK);

    ems[i] = (const char *) Tcl_GetStringFromObj (elm, NULL);
  }

  CORBA::EnumDef_var ed;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    ed = c->create_enum (id, name, version, ems);
    assert (!CORBA::is_nil (ed));
  }
  else {
    ed = CORBA::EnumDef::_narrow (cv);
    if (CORBA::is_nil (ed)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding enum \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not an enum",
			NULL);
      return TCL_ERROR;
    }
    ed->id (id);
    ed->version (version);
    ed->members (ems);
  }

  return TCL_OK;
}

int
CodeScanTcl::addAlias (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3;
  const char *str;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 3 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "typedef") != 0) {
    Tcl_AppendResult (interp, "error: not a typedef: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like typedef data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning typedef from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::IDLType_var itv = scanIDLTypeName (o3);

  if (CORBA::is_nil (itv)) {
    Tcl_AppendResult (interp, "\n  while scanning typedef from \"",
		      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::AliasDef_var ad;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    ad = c->create_alias (id, name, version, itv);
    assert (!CORBA::is_nil (ad));
  }
  else {
    ad = CORBA::AliasDef::_narrow (cv);
    if (CORBA::is_nil (ad)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding typedef \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a typedef",
			NULL);
      return TCL_ERROR;
    }
    ad->id (id);
    ad->version (version);
    ad->original_type_def (itv);
  }

  return TCL_OK;
}

int
CodeScanTcl::addAttribute (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *name, *id, *version;
  Tcl_Obj *o1, *o2, *o3;
  const char *str;
  int len;

  CORBA::InterfaceDef_var idv = CORBA::InterfaceDef::_narrow (c);
  CORBA::ValueDef_var vdv = CORBA::ValueDef::_narrow (c);
  assert (!CORBA::is_nil (idv) || !CORBA::is_nil (vdv));

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
      (len != 3 && len != 4) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "attribute") != 0) {
    Tcl_AppendResult (interp, "error: not an attribute: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like attribute data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning attribute from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::IDLType_var rtype = scanIDLTypeName (o3);

  if (CORBA::is_nil (rtype)) {
    Tcl_AppendResult (interp, "\n  while scanning attribute type from \"",
		      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::AttributeMode mode = CORBA::ATTR_NORMAL;

  if (len == 4) {
    Tcl_Obj *o4;
    const char *ro;
    int r;
    r = Tcl_ListObjIndex (NULL, data, 3, &o4);
    assert (r == TCL_OK);
    ro = Tcl_GetStringFromObj (o4, NULL);
    if (strcmp (ro, "readonly") == 0) {
      mode = CORBA::ATTR_READONLY;
    }
  }

  CORBA::AttributeDef_var adv;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    if (!CORBA::is_nil (idv)) {
      adv = idv->create_attribute (id, name, version, rtype, mode);
    }
    else {
      adv = vdv->create_attribute (id, name, version, rtype, mode);
    }
    assert (!CORBA::is_nil (adv));
  }
  else {
    adv = CORBA::AttributeDef::_narrow (cv);
    if (CORBA::is_nil (adv)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding attribute \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not an attribute",
			NULL);
      return TCL_ERROR;
    }
    adv->id (id);
    adv->version (version);
    adv->type_def (rtype);
    adv->mode (mode);
  }

  return TCL_OK;
}

int
CodeScanTcl::addOperation (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *name, *id, *version;
  Tcl_Obj *o1, *o2, *o3, *o4, *o5;
  int len, npars, nexs, i;
  const char *str;

  CORBA::InterfaceDef_var idv = CORBA::InterfaceDef::_narrow (c);
  CORBA::ValueDef_var vdv = CORBA::ValueDef::_narrow (c);
  assert (!CORBA::is_nil (idv) || !CORBA::is_nil (vdv));

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
      (len != 5 && len != 6) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 4, &o5) != TCL_OK ||
      Tcl_ListObjLength (NULL, o4, &npars) != TCL_OK ||
      Tcl_ListObjLength (NULL, o5, &nexs) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "operation") != 0) {
    Tcl_AppendResult (interp, "error: not an operation: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like operation data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning operation from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::IDLType_var rtype = scanIDLTypeName (o3);

  if (CORBA::is_nil (rtype)) {
    Tcl_AppendResult (interp, "\n  while scanning operation ",
		      "return type from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Scan Parameters
   */

  CORBA::ParDescriptionSeq pars (npars);
  pars.length (npars);

  for (i=0; i<npars; i++) {
    Tcl_Obj *pobj, *pmo, *pno, *pto;
    const char *pname, *pmode;
    int count, r;

    r = Tcl_ListObjIndex (NULL, o4, i, &pobj);
    assert (r == TCL_OK);

    if (Tcl_ListObjLength (NULL, pobj, &count) != TCL_OK || count != 3 ||
	Tcl_ListObjIndex (NULL, pobj, 0, &pmo) != TCL_OK ||
	Tcl_ListObjIndex (NULL, pobj, 1, &pno) != TCL_OK ||
	Tcl_ListObjIndex (NULL, pobj, 2, &pto) != TCL_OK) {
      Tcl_AppendResult (interp, "error: cannot read operation parameter from \"",
			Tcl_GetStringFromObj (pobj, NULL),
			"\" (should be <mode> <name> <type>)", NULL);
      Tcl_AppendResult (interp, "\n  while scanning operation from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    pmode = Tcl_GetStringFromObj (pmo, NULL);
    pname = Tcl_GetStringFromObj (pno, NULL);
    CORBA::IDLType_var pt = scanIDLTypeName (pto);

    if (CORBA::is_nil (pt)) {
      Tcl_AppendResult (interp, "\n  while scanning operation from \"",
			Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return TCL_ERROR;
    }

    if (strcmp (pmode, "in") == 0) {
      pars[i].mode = CORBA::PARAM_IN;
    }
    else if (strcmp (pmode, "out") == 0) {
      pars[i].mode = CORBA::PARAM_OUT;
    }
    else if (strcmp (pmode, "inout") == 0) {
      pars[i].mode = CORBA::PARAM_INOUT;
    }
    else {
      Tcl_AppendResult (interp, "error: cannot read operation mode from \"",
			pmode, "\": should be in, out or inout", NULL);
      Tcl_AppendResult (interp, "\n  while scanning operation from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }

    pars[i].name     = pname;
    pars[i].type     = pt->type();
    pars[i].type_def = CORBA::IDLType::_duplicate (pt);
  }

  /*
   * Scan exceptions
   */

  CORBA::ExceptionDefSeq eds (nexs);
  eds.length (nexs);

  for (i=0; i<nexs; i++) {
    Tcl_Obj *exo;
    int r;

    r = Tcl_ListObjIndex (NULL, o5, i, &exo);
    assert (r == TCL_OK);

    const char *exn = Tcl_GetStringFromObj (exo, NULL);
    CORBA::Contained_var cv = repo->lookup_id (exn);
    CORBA::ExceptionDef_var ex = CORBA::ExceptionDef::_narrow (cv);

    if (CORBA::is_nil (ex)) {
      Tcl_AppendResult (interp, "error: operation \"", id,
			"\" generates unknown exception \"",
			exn, "\"", NULL);
      Tcl_AppendResult (interp, "\n  while scanning operation from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
      return TCL_ERROR;
    }
    eds[i] = CORBA::ExceptionDef::_duplicate (ex);
  }

  CORBA::OperationMode mode = CORBA::OP_NORMAL;

  if (len == 6) {
    Tcl_Obj *o6;
    const char *ow;
    int r;
    r = Tcl_ListObjIndex (NULL, data, 5, &o6);
    assert (r == TCL_OK);
    ow = Tcl_GetStringFromObj (o6, NULL);
    if (strcmp (ow, "oneway") == 0) {
      mode = CORBA::OP_ONEWAY;
    }
  }

  CORBA::ContextIdSeq cis;

  CORBA::OperationDef_var od;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    if (!CORBA::is_nil (idv)) {
      od = idv->create_operation (id, name, version,
				  rtype, mode, pars,
				  eds, cis);
    }
    else {
      od = vdv->create_operation (id, name, version,
				  rtype, mode, pars,
				  eds, cis);
    }
    assert (!CORBA::is_nil (od));
  }
  else {
    od = CORBA::OperationDef::_narrow (cv);
    if (CORBA::is_nil (od)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding operation \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not an operation",
			NULL);
      return TCL_ERROR;
    }
    od->id (id);
    od->version (version);
    od->result_def (rtype);
    od->params (pars);
    od->mode (mode);
    od->contexts (cis);
    od->exceptions (eds);
  }

  return TCL_OK;
}

int
CodeScanTcl::addInterface (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3, *o4;
  const char *str;
  int len, nbase;

  /*
   * Can either be
   *   interface repoid base_interfaces contained_decl
   * or
   *   interface repoid
   * for forward declarations
   */

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || 
      (len != 2 && len != 4) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (len == 4 && Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK) ||
      (len == 4 && Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK) ||
      (len == 4 && Tcl_ListObjLength (NULL, o3, &nbase) != TCL_OK) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (strcmp (str, "interface") != 0 &&
       strcmp (str, "localinterface") != 0 &&
       strcmp (str, "abstractinterface") != 0)) {
    Tcl_AppendResult (interp, "error: not an interface: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like interface data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning interface from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Create interface
   */

  CORBA::Contained_var cv = lookup_local (name, c);
  CORBA::InterfaceDef_var ni;
  CORBA::InterfaceDefSeq bases;
  bases.length (0);

  if (CORBA::is_nil (cv)) {
#if !defined(COMBAT_USE_MICO) || MICO_BIN_VERSION > 0x020305
    ni = c->create_interface (id, name, version, bases);
#else
    ni = c->create_interface (id, name, version, bases, 0);
#endif
  }
  else {
    ni = CORBA::InterfaceDef::_narrow (cv);
    if (CORBA::is_nil (ni)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding interface \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not an interface",
			NULL);
      return TCL_ERROR;
    }
    ni->id (id);
    ni->version (version);
  }

  /*
   * Forward reference only?
   */

  if (len == 2) {
    return TCL_OK;
  }

  /*
   * Scan Base Interfaces
   */

  bases.length (nbase);

  for (int i=0; i<nbase; i++) {
    Tcl_Obj *bobj;
    int r;
      
    r = Tcl_ListObjIndex (NULL, o3, i, &bobj);
    assert (r == TCL_OK);
      
    CORBA::IDLType_var bt = scanIDLTypeName (bobj);
      
    if (CORBA::is_nil (bt)) {
      Tcl_AppendResult (interp, "\n  while scanning base interfaces from\"",
			Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return TCL_ERROR;
    }
      
    CORBA::InterfaceDef_var bi = CORBA::InterfaceDef::_narrow (bt);
    assert (!CORBA::is_nil (bi));
    bases[i] = CORBA::InterfaceDef::_duplicate (bi);
  }

  ni->base_interfaces (bases);

  /*
   * Scan members
   */

  if (addContainer (o4, ni) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning interface from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
CodeScanTcl::addValue (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3, *o4, *o5, *o6, *o7, *o8;
  int len, nbase, nsupp, nmods, ninits;
  const char *str;

  /*
   * Can either be
   *   valuetype repoid base bases supported inits modifiers contained_decl
   * or
   *   valuetype repoid
   * for forward declarations
   */

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || 
      (len != 2 && len != 8) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (len == 8 && Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK) ||
      (len == 8 && Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK) ||
      (len == 8 && Tcl_ListObjIndex (NULL, data, 4, &o5) != TCL_OK) ||
      (len == 8 && Tcl_ListObjIndex (NULL, data, 5, &o6) != TCL_OK) ||
      (len == 8 && Tcl_ListObjIndex (NULL, data, 6, &o7) != TCL_OK) ||
      (len == 8 && Tcl_ListObjIndex (NULL, data, 7, &o8) != TCL_OK) ||
      (len == 8 && Tcl_ListObjLength (NULL, o4, &nbase) != TCL_OK) ||
      (len == 8 && Tcl_ListObjLength (NULL, o5, &nsupp) != TCL_OK) ||
      (len == 8 && Tcl_ListObjLength (NULL, o6, &ninits) != TCL_OK) ||
      (len == 8 && Tcl_ListObjLength (NULL, o7, &nmods) != TCL_OK) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "valuetype") != 0) {
    Tcl_AppendResult (interp, "error: not a valuetype: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like valuetype data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning valuetype from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  /*
   * Create value
   */

  CORBA::Contained_var cv = lookup_local (name, c);
  CORBA::InterfaceDefSeq supported;
  CORBA::InitializerSeq inits;
  CORBA::ValueDefSeq bases;
  CORBA::ValueDef_var base, ni;

  base = CORBA::ValueDef::_nil ();
  supported.length (0);
  bases.length (0);

  if (CORBA::is_nil (cv)) {
    ni = c->create_value (id, name, version, 0, 0,
			  base, 0, bases, supported,
			  inits);
  }
  else {
    ni = CORBA::ValueDef::_narrow (cv);
    if (CORBA::is_nil (ni)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding valuetype \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a valuetype",
			NULL);
      return TCL_ERROR;
    }
    ni->id (id);
    ni->version (version);
  }

  /*
   * Forward reference only?
   */

  if (len == 2) {
    return TCL_OK;
  }

  /*
   * Scan concrete base value
   */

  str = Tcl_GetStringFromObj (o3, NULL);

  if (strcmp (str, "0") != 0) {
    CORBA::IDLType_var bt = scanIDLTypeName (o3);

    if (CORBA::is_nil (bt)) {
      Tcl_AppendResult (interp, "\n  while scanning base value from\"",
			Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return TCL_ERROR;
    }
      
    base = CORBA::ValueDef::_narrow (bt);
    assert (!CORBA::is_nil (base));
    ni->base_value (base);
  }

  /*
   * Scan abstract base values
   */

  bases.length (nbase);

  for (int i=0; i<nbase; i++) {
    Tcl_Obj *bobj;
    int r;
      
    r = Tcl_ListObjIndex (NULL, o4, i, &bobj);
    assert (r == TCL_OK);
      
    CORBA::IDLType_var bt = scanIDLTypeName (bobj);
      
    if (CORBA::is_nil (bt)) {
      Tcl_AppendResult (interp, "\n  while scanning base values from\"",
			Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return TCL_ERROR;
    }
      
    CORBA::ValueDef_var bi = CORBA::ValueDef::_narrow (bt);
    assert (!CORBA::is_nil (bi));
    bases[i] = CORBA::ValueDef::_duplicate (bi);
  }

  ni->abstract_base_values (bases);

  /*
   * Scan supported interfaces
   */

  supported.length (nsupp);

  for (int i=0; i<nsupp; i++) {
    Tcl_Obj *bobj;
    int r;
      
    r = Tcl_ListObjIndex (NULL, o5, i, &bobj);
    assert (r == TCL_OK);
      
    CORBA::IDLType_var bt = scanIDLTypeName (bobj);
      
    if (CORBA::is_nil (bt)) {
      Tcl_AppendResult (interp, "\n  while scanning supported interfaces from\"",
			Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      return TCL_ERROR;
    }
      
    CORBA::InterfaceDef_var bi = CORBA::InterfaceDef::_narrow (bt);
    assert (!CORBA::is_nil (bi));
    supported[i] = CORBA::InterfaceDef::_duplicate (bi);
  }

  ni->supported_interfaces (supported);

  /*
   * Scan initializers
   */

  inits.length (ninits);

  for (int i=0; i<ninits; i++) {
    Tcl_Obj *iobj, *io1, *io2;
    int r, ilen, nmems;
    const char *iname;

    if (Tcl_ListObjIndex (NULL, o6, i, &iobj) != TCL_OK ||
	Tcl_ListObjLength (NULL, iobj, &ilen) != TCL_OK || (ilen != 2) ||
	Tcl_ListObjIndex (NULL, iobj, 0, &io1) != TCL_OK ||
	Tcl_ListObjIndex (NULL, iobj, 1, &io2) != TCL_OK ||
	Tcl_ListObjLength (NULL, io2, &nmems) != TCL_OK ||
	(iname = Tcl_GetStringFromObj (io1, NULL)) == NULL) {
      Tcl_AppendResult (interp, "error: cannot get initializer data: \"",
			Tcl_GetStringFromObj (o6, NULL),
			"\" (does not look like initializer data)", NULL);
      return TCL_ERROR;
    }

    inits[i].name = iname;
    inits[i].members.length (nmems);
    
    for (int j=0; j<nmems; j++) {
      Tcl_Obj *mobj, *mno, *mto;
      const char *mname;
      int count;

      r = Tcl_ListObjIndex (NULL, io2, j, &mobj);
      assert (r == TCL_OK);

      if (Tcl_ListObjLength (NULL, mobj, &count) != TCL_OK || count != 2 ||
	  Tcl_ListObjIndex (NULL, mobj, 0, &mno) != TCL_OK ||
	  Tcl_ListObjIndex (NULL, mobj, 1, &mto) != TCL_OK) {
	Tcl_AppendResult (interp, "error: cannot read struct member from \"",
			  Tcl_GetStringFromObj (mobj, NULL),
			  "\" (should be <name> <type>)", NULL);
	Tcl_AppendResult (interp, "\n  while scanning value initializers from \"",
			  Tcl_GetStringFromObj (o6, NULL),
			  "\"", NULL);
	return TCL_ERROR;
      }

      mname = Tcl_GetStringFromObj (mno, NULL);
      CORBA::IDLType_var itv = scanIDLTypeName (mto);

      if (CORBA::is_nil (itv)) {
	Tcl_AppendResult (interp, "\n  while scanning struct member from \"",
			  Tcl_GetStringFromObj (mobj, NULL), "\"", NULL);
	Tcl_AppendResult (interp, "\n  while scanning value initializers from \"",
			  Tcl_GetStringFromObj (o6, NULL),
			  "\"", NULL);
	return TCL_ERROR;
      }

      inits[i].members[j].name = mname;
      inits[i].members[j].type = CORBA::TypeCode::_duplicate (CORBA::_tc_void);
      inits[i].members[j].type_def = CORBA::IDLType::_duplicate (itv);
    }
  }

  ni->initializers (inits);

  /*
   * Scan modifiers
   */

  for (int i=0; i<nmods; i++) {
    const char * mod;
    Tcl_Obj * mobj;
    int r;

    r = Tcl_ListObjIndex (NULL, o7, i, &mobj);
    assert (r == TCL_OK);
    mod = Tcl_GetStringFromObj (mobj, NULL);
    assert (mod != NULL);

    if (strcmp (mod, "custom") == 0) {
      ni->is_custom (1);
    }
    else if (strcmp (mod, "abstract") == 0) {
      ni->is_abstract (1);
    }
    else if (strcmp (mod, "truncatable") == 0) {
      ni->is_truncatable (1);
    }
    else {
      assert (0);
    }
  }

  /*
   * Scan members
   */

  if (addContainer (o8, ni) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning interface from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
CodeScanTcl::addValueBox (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2, *o3;
  const char *str;
  int len;

  /*
   * valuebox repoid original_type_def
   */

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 3 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "valuebox") != 0) {
    Tcl_AppendResult (interp, "error: not a valuebox: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like valuebox data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning typedef from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::IDLType_var itv = scanIDLTypeName (o3);

  if (CORBA::is_nil (itv)) {
    Tcl_AppendResult (interp, "\n  while scanning valuebox from \"",
		      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::ValueBoxDef_var ad;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    ad = c->create_value_box (id, name, version, itv);
    assert (!CORBA::is_nil (ad));
  }
  else {
    ad = CORBA::ValueBoxDef::_narrow (cv);
    if (CORBA::is_nil (ad)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding valuebox \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a valuebox",
			NULL);
      return TCL_ERROR;
    }
    ad->id (id);
    ad->version (version);
    ad->original_type_def (itv);
  }

  return TCL_OK;
}

int
CodeScanTcl::addValueMember (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *name, *id, *version;
  Tcl_Obj *o1, *o2, *o3, *o4;
  const char *str, *visi;
  int len;

  /*
   * valuemember repoid type_def visibility
   */

  CORBA::ValueDef_var idv = CORBA::ValueDef::_narrow (c);
  assert (!CORBA::is_nil (idv));

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 4) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (visi = Tcl_GetStringFromObj (o4, NULL)) == NULL ||
      strcmp (str, "valuemember") != 0) {
    Tcl_AppendResult (interp, "error: not a valuemember: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like valuemember data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning valuemember from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::IDLType_var rtype = scanIDLTypeName (o3);

  if (CORBA::is_nil (rtype)) {
    Tcl_AppendResult (interp, "\n  while scanning attribute type from \"",
		      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::Visibility mode;

  if (strcmp (visi, "private") == 0) {
    mode = CORBA::PRIVATE_MEMBER;
  }
  else {
    mode = CORBA::PUBLIC_MEMBER;
  }

  CORBA::ValueMemberDef_var adv;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    adv = idv->create_value_member (id, name, version, rtype, mode);
    assert (!CORBA::is_nil (adv));
  }
  else {
    adv = CORBA::ValueMemberDef::_narrow (cv);
    if (CORBA::is_nil (adv)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding valuemember \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a valuemember",
			NULL);
      return TCL_ERROR;
    }
    adv->id (id);
    adv->version (version);
    adv->type_def (rtype);
    adv->access (mode);
  }

  return TCL_OK;
}


int
CodeScanTcl::addNative (Tcl_Obj * data, CORBA::Container_ptr c)
{
  char *id, *name, *version;
  Tcl_Obj *o1, *o2;
  const char *str;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "native") != 0) {
    Tcl_AppendResult (interp, "error: not a native: \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\" (does not look like native data)", NULL);
    return TCL_ERROR;
  }

  if (scanRepoid (o2, id, name, version) != TCL_OK) {
    Tcl_AppendResult (interp, "\n  while scanning native from \"",
		      Tcl_GetStringFromObj (data, NULL),
		      "\"", NULL);
    return TCL_ERROR;
  }

  CORBA::NativeDef_var nd;
  CORBA::Contained_var cv = lookup_local (name, c);

  if (CORBA::is_nil (cv)) {
    nd = c->create_native (id, name, version);
    assert (!CORBA::is_nil (nd));
  }
  else {
    nd = CORBA::NativeDef::_narrow (cv);
    if (CORBA::is_nil (nd)) {
      CORBA::String_var frame = absolute_name (c);
      Tcl_AppendResult (interp, "error: while adding module \"",
			name, "\" to \"", (char *) frame,
			"\": identifier exists, but is not a native",
			NULL);
      return TCL_ERROR;
    }
    nd->id (id);
    nd->version (version);
  }

  return TCL_OK;
}
