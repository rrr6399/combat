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
 * Handling of Typecode values
 * ----------------------------------------------------------------------
 */

#include "combat.h"
#include <vector>
#include <set>
#include <string>
#include <assert.h>

char * combat_typecode_id = "$Id$";

/*
 * TypeCode constructing / deconstructing classes
 */

class TypeCodeGenTcl {
public:
  TypeCodeGenTcl (void);
  Tcl_Obj * emit (const CORBA::TypeCode_ptr);

private:
  Tcl_Obj * emitObjref    (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitString    (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitWstring   (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitSequence  (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitArray     (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitFixed     (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitStruct    (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitUnion     (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitEnum      (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitAlias     (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitException (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitValue     (const CORBA::TypeCode_ptr);
  Tcl_Obj * emitValueBox  (const CORBA::TypeCode_ptr);

#if !defined(COMBAT_USE_MICO) || !defined(HAVE_MINI_STL)
  std::set<std::string> recursion;
#else
  set<string,less<string> > recursion;
#endif
};

class TypeCodeScanTcl {
public:
  TypeCodeScanTcl (Tcl_Interp *);
  CORBA::TypeCode_ptr scan (Tcl_Obj *);

private:
  Tcl_Interp * interp;

  CORBA::TypeCode_ptr scanObjref    (Tcl_Obj *);
  CORBA::TypeCode_ptr scanString    (Tcl_Obj *);
  CORBA::TypeCode_ptr scanWstring   (Tcl_Obj *);
  CORBA::TypeCode_ptr scanSequence  (Tcl_Obj *);
  CORBA::TypeCode_ptr scanRecursive (Tcl_Obj *);
  CORBA::TypeCode_ptr scanArray     (Tcl_Obj *);
  CORBA::TypeCode_ptr scanFixed     (Tcl_Obj *);
  CORBA::TypeCode_ptr scanStruct    (Tcl_Obj *);
  CORBA::TypeCode_ptr scanUnion     (Tcl_Obj *);
  CORBA::TypeCode_ptr scanEnum      (Tcl_Obj *);
  CORBA::TypeCode_ptr scanException (Tcl_Obj *);
  CORBA::TypeCode_ptr scanValue     (Tcl_Obj *);
  CORBA::TypeCode_ptr scanValueBox  (Tcl_Obj *);
};

/*
 * ----------------------------------------------------------------------
 * Registration of CORBA::TypeCode as a Tcl_Obj type
 * ----------------------------------------------------------------------
 */

extern "C" {

static int
TclTypeCode_SetFromAny (Tcl_Interp * interp, Tcl_Obj * obj)
{
  if (obj->typePtr == &Combat::TypeCodeType) {
    return TCL_OK;
  }

  TypeCodeScanTcl tcst (interp);
  CORBA::TypeCode_ptr tc = tcst.scan (obj);

  if (CORBA::is_nil (tc)) {
    return TCL_ERROR;
  }

  if (obj->typePtr != NULL && obj->typePtr->freeIntRepProc != NULL) {
    obj->typePtr->freeIntRepProc (obj);
  }

  obj->typePtr = &Combat::TypeCodeType;
  obj->internalRep.otherValuePtr = (VOID *) (void *) tc;

  return TCL_OK;
}

static void
TclTypeCode_UpdateString (Tcl_Obj * obj)
{
  assert (obj->typePtr == &Combat::TypeCodeType);
  CORBA::TypeCode_ptr tc =
    (CORBA::TypeCode *) (void *) obj->internalRep.otherValuePtr;

  TypeCodeGenTcl tcgt;
  Tcl_Obj * res = tcgt.emit (tc);

  /*
   * Force object to update string rep
   */

  if (res->bytes == NULL) {
    res->typePtr->updateStringProc (res);
  }

  /*
   * steal its string rep
   */

  obj->bytes  = res->bytes;
  obj->length = res->length;
  res->bytes  = NULL;

  Tcl_DecrRefCount (res);
}

static void
TclTypeCode_DupInternal (Tcl_Obj * src, Tcl_Obj * dup)
{
  assert (src->typePtr == &Combat::TypeCodeType);
  CORBA::TypeCode_ptr tc =
    (CORBA::TypeCode *) (void *) src->internalRep.otherValuePtr;

  dup->typePtr = src->typePtr;
  dup->internalRep.otherValuePtr =
    (VOID *) (void *) CORBA::TypeCode::_duplicate (tc);
}

static void
TclTypeCode_FreeInternal (Tcl_Obj * obj)
{
  assert (obj->typePtr == &Combat::TypeCodeType);
  CORBA::TypeCode_ptr tc =
    (CORBA::TypeCode *) (void *) obj->internalRep.otherValuePtr;
  CORBA::release (tc);
}

} // extern "C"

#ifdef HAVE_NAMESPACE
namespace Combat {
  Tcl_ObjType TypeCodeType = {
    "CORBA::TypeCode",
    TclTypeCode_FreeInternal,
    TclTypeCode_DupInternal,
    TclTypeCode_UpdateString,
    TclTypeCode_SetFromAny
  };
};
#else
Tcl_ObjType Combat::TypeCodeType = {
  "CORBA::TypeCode",
  TclTypeCode_FreeInternal,
  TclTypeCode_DupInternal,
  TclTypeCode_UpdateString,
  TclTypeCode_SetFromAny
};
#endif

/*
 * Create a new TypeCode object. The TypeCode is not consumed
 */

Tcl_Obj *
Combat::NewTypeCodeObj (CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * obj = Tcl_NewObj ();
  Tcl_InvalidateStringRep (obj);

  obj->typePtr = &TypeCodeType;
  obj->internalRep.otherValuePtr =
    (VOID *) (void *) CORBA::TypeCode::_duplicate (tc);

  return obj;
}

CORBA::TypeCode_ptr
Combat::GetTypeCodeFromObj (Tcl_Interp * interp, Tcl_Obj * data)
{
  /*
   * If data is already a TypeCode object, use the internal rep
   */

  if (Tcl_ConvertToType (interp, data, &TypeCodeType) != TCL_OK) {
    return CORBA::TypeCode::_nil ();
  }

  assert (data->typePtr == &TypeCodeType);

  CORBA::TypeCode_ptr tc =
    (CORBA::TypeCode *) (void *) data->internalRep.otherValuePtr;
  return CORBA::TypeCode::_duplicate (tc);
}

/*
 * ----------------------------------------------------------------------
 * Describe a TypeCode
 * ----------------------------------------------------------------------
 */

TypeCodeGenTcl::TypeCodeGenTcl (void)
{
  // nothing to do
}

Tcl_Obj *
TypeCodeGenTcl::emit (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * res = NULL;

  /*
   * Detect and break recursion.
   */

  switch (tc->kind()) {
  case CORBA::tk_struct:
  case CORBA::tk_union:
  case CORBA::tk_value:
    {
      if (recursion.find (tc->id()) != recursion.end()) {
	Tcl_Obj * o[2];
	o[0] = Tcl_NewStringObj ("recursive", 9);
	o[1] = Tcl_NewStringObj ((char *) tc->id(), -1);
	return Tcl_NewListObj (2, o);
      }
      recursion.insert (tc->id());
    }
  }

  switch (tc->kind()) {
  case CORBA::tk_null:
    res = Tcl_NewStringObj ("null", 4);
    break;

  case CORBA::tk_void:
    res = Tcl_NewStringObj ("void", 4);
    break;

  case CORBA::tk_short:
    res = Tcl_NewStringObj ("short", 5);
    break;

  case CORBA::tk_long:
    res = Tcl_NewStringObj ("long", 4);
    break;

  case CORBA::tk_ushort:
    res = Tcl_NewStringObj ("unsigned short", 14);
    break;

  case CORBA::tk_ulong:
    res = Tcl_NewStringObj ("unsigned long", 13);
    break;

  case CORBA::tk_float:
    res = Tcl_NewStringObj ("float", 5);
    break;

  case CORBA::tk_double:
    res = Tcl_NewStringObj ("double", 6);
    break;

  case CORBA::tk_boolean:
    res = Tcl_NewStringObj ("boolean", 7);
    break;

  case CORBA::tk_char:
    res = Tcl_NewStringObj ("char", 4);
    break;

  case CORBA::tk_octet:
    res = Tcl_NewStringObj ("octet", 5);
    break;

  case CORBA::tk_any:
    res = Tcl_NewStringObj ("any", 3);
    break;

  case CORBA::tk_TypeCode:
    res = Tcl_NewStringObj ("TypeCode", 8);
    break;

  case CORBA::tk_Principal:
    res = Tcl_NewStringObj ("Principal", 9);
    break;

  case CORBA::tk_longlong:
    res = Tcl_NewStringObj ("long long", 9);
    break;

  case CORBA::tk_ulonglong:
    res = Tcl_NewStringObj ("unsigned long long", 18);
    break;

  case CORBA::tk_longdouble:
    res = Tcl_NewStringObj ("long double", 11);
    break;

  case CORBA::tk_wchar:
    res = Tcl_NewStringObj ("wchar", 5);
    break;

    /*
     * Constructed types
     */

  case CORBA::tk_objref:
    res = emitObjref (tc);
    break;

  case CORBA::tk_string:
    res = emitString (tc);
    break;

  case CORBA::tk_wstring:
    res = emitWstring (tc);
    break;

  case CORBA::tk_sequence:
    res = emitSequence (tc);
    break;

  case CORBA::tk_array:
    res = emitArray (tc);
    break;

  case CORBA::tk_fixed:
    res = emitFixed (tc);
    break;

    /*
     * Complex types
     */

  case CORBA::tk_struct:
    res = emitStruct (tc);
    break;

  case CORBA::tk_union:
    res = emitUnion (tc);
    break;

  case CORBA::tk_enum:
    res = emitEnum (tc);
    break;

  case CORBA::tk_alias:
    res = emitAlias (tc);
    break;

  case CORBA::tk_except:
    res = emitException (tc);
    break;

  case CORBA::tk_value:
    res = emitValue (tc);
    break;

  case CORBA::tk_value_box:
    res = emitValueBox (tc);
    break;

  default:
    assert (0);
  }

  switch (tc->kind()) {
  case CORBA::tk_struct:
  case CORBA::tk_union:
  case CORBA::tk_value:
    {
      recursion.erase (tc->id());
    }
  }

  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitObjref (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * res = Tcl_NewStringObj ("Object", 6);

  if (strcmp (tc->id(), "IDL:omg.org/CORBA/Object:1.0") != 0) {
    Tcl_Obj * to = Tcl_NewStringObj ((char *) tc->id(), -1);
    Tcl_ListObjAppendElement (NULL, res, to);
  }

  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitString (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * res = Tcl_NewStringObj ("string", 6);

  if (tc->length()) {
    char tmp[256];
    sprintf (tmp, "%lu", (unsigned long) tc->length());
    Tcl_Obj * to = Tcl_NewStringObj (tmp, -1);
    Tcl_ListObjAppendElement (NULL, res, to);
  }

  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitWstring (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * res = Tcl_NewStringObj ("wstring", 7);

  if (tc->length()) {
    char tmp[256];
    sprintf (tmp, "%lu", (unsigned long) tc->length());
    Tcl_Obj * to = Tcl_NewStringObj (tmp, -1);
    Tcl_ListObjAppendElement (NULL, res, to);
  }

  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitSequence (const CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ntc = tc->content_type();
  Tcl_Obj *s[2], *res;

  s[0] = Tcl_NewStringObj ("sequence", 8);
  s[1] = emit (ntc);
  res  = Tcl_NewListObj (2, s);

  if (tc->length()) {
    char tmp[256];
    sprintf (tmp, "%lu", (unsigned long) tc->length());
    Tcl_Obj * to = Tcl_NewStringObj (tmp, -1);
    Tcl_ListObjAppendElement (NULL, res, to);
  }

  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitArray (const CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ntc = tc->content_type();
  Tcl_Obj *a[3], *res;
  char tmp[256];

  sprintf (tmp, "%lu", (unsigned long) tc->length());

  a[0] = Tcl_NewStringObj ("array", 5);
  a[1] = emit (ntc);
  a[2] = Tcl_NewStringObj (tmp, -1);

  res  = Tcl_NewListObj (3, a);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitFixed (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * s[3], * res;

  CORBA::Any a1, a2;
  a1 <<= tc->fixed_digits();
  a2 <<= tc->fixed_scale();

  s[0] = Tcl_NewStringObj ("fixed", 5);
  s[1] = Combat::NewAnyObj (a1);
  s[2] = Combat::NewAnyObj (a2);

  res  = Tcl_NewListObj (3, s);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitStruct (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj *o[3], *res;

  o[0] = Tcl_NewStringObj ("struct", 6);
  o[1] = Tcl_NewStringObj ((char *) tc->id(), -1);
  o[2] = Tcl_NewObj ();

  for (CORBA::ULong i=0; i<tc->member_count(); i++) {
    Tcl_Obj *m[2];
    CORBA::TypeCode_var mtc = tc->member_type (i);
    m[0] = Tcl_NewStringObj ((char *) tc->member_name (i), -1);
    m[1] = emit (mtc);
    Tcl_ListObjAppendElement (NULL, o[2], m[0]);
    Tcl_ListObjAppendElement (NULL, o[2], m[1]);
  }

  res = Tcl_NewListObj (3, o);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitUnion (const CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var dtype = tc->discriminator_type ();
  CORBA::Long defidx = tc->default_index ();
  Tcl_Obj *o[4], *res;
  
  o[0] = Tcl_NewStringObj ("union", 5);
  o[1] = Tcl_NewStringObj ((char *) tc->id(), -1);
  o[2] = emit (dtype);
  o[3] = Tcl_NewObj ();

  for (CORBA::ULong i=0; i<tc->member_count(); i++) {
    CORBA::TypeCode_var mtc = tc->member_type (i);
    Tcl_Obj *m[2];
    
    if (defidx != -1 && i == (CORBA::ULong) defidx) {
      m[0] = Tcl_NewStringObj ("(default)", 9);
    }
    else {
      CORBA::Any_var label = tc->member_label (i);
      m[0] = Combat::NewAnyObj (label.in());
    }
    m[1] = emit (mtc);
    Tcl_ListObjAppendElement (NULL, o[3], m[0]);
    Tcl_ListObjAppendElement (NULL, o[3], m[1]);
  }

  res = Tcl_NewListObj (4, o);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitEnum (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj *o[2], *res;

  o[0] = Tcl_NewStringObj ("enum", 4);
  o[1] = Tcl_NewObj ();
  for (CORBA::ULong i=0; i<tc->member_count(); i++) {
    const char * elm = tc->member_name (i);
    Tcl_Obj *elo = Tcl_NewStringObj ((char *) elm, -1);
    Tcl_ListObjAppendElement (NULL, o[1], elo);
  }

  res = Tcl_NewListObj (2, o);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitAlias (const CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ctc = tc->content_type();
  return emit (ctc);
}

Tcl_Obj *
TypeCodeGenTcl::emitException (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj *o[3], *res;

  o[0] = Tcl_NewStringObj ("exception", 9);
  o[1] = Tcl_NewStringObj ((char *) tc->id(), -1);
  o[2] = Tcl_NewObj ();

  for (CORBA::ULong i=0; i<tc->member_count(); i++) {
    Tcl_Obj *m[2];
    CORBA::TypeCode_var mtc = tc->member_type (i);
    m[0] = Tcl_NewStringObj ((char *) tc->member_name (i), -1);
    m[1] = emit (mtc);
    Tcl_ListObjAppendElement (NULL, o[2], m[0]);
    Tcl_ListObjAppendElement (NULL, o[2], m[1]);
  }

  res = Tcl_NewListObj (3, o);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitValue (const CORBA::TypeCode_ptr tc)
{
  Tcl_Obj *o[5], *res;

  o[0] = Tcl_NewStringObj ("valuetype", 9);
  o[1] = Tcl_NewStringObj ((char *) tc->id(), -1);
  o[2] = Tcl_NewObj ();

  for (CORBA::ULong i=0; i<tc->member_count(); i++) {
    Tcl_Obj *m[3];
    CORBA::TypeCode_var mtc = tc->member_type (i);

    if (tc->member_visibility (i) == CORBA::PRIVATE_MEMBER) {
      m[0] = Tcl_NewStringObj ("private", 7);
    }
    else {
      m[0] = Tcl_NewStringObj ("public", 6);
    }

    m[1] = Tcl_NewStringObj ((char *) tc->member_name (i), -1);
    m[2] = emit (mtc);

    Tcl_ListObjAppendElement (NULL, o[2], m[0]);
    Tcl_ListObjAppendElement (NULL, o[2], m[1]);
    Tcl_ListObjAppendElement (NULL, o[2], m[2]);
  }

  CORBA::TypeCode_var base = tc->concrete_base_type ();

  if (CORBA::is_nil (base)) {
    o[3] = Tcl_NewIntObj (0);
  }
  else {
    o[3] = emit (base);
  }

  if (tc->type_modifier() == CORBA::VM_NONE) {
    o[4] = Tcl_NewObj ();
  }
  else if (tc->type_modifier() == CORBA::VM_CUSTOM) {
    o[4] = Tcl_NewStringObj ("custom", 6);
  }
  else if (tc->type_modifier() == CORBA::VM_ABSTRACT) {
    o[4] = Tcl_NewStringObj ("abstract", 8);
  }
  else if (tc->type_modifier() == CORBA::VM_TRUNCATABLE) {
    o[4] = Tcl_NewStringObj ("truncatable", 11);
  }
  else {
    o[4] = Tcl_NewObj ();
    assert (0);
  }

  res = Tcl_NewListObj (5, o);
  return res;
}

Tcl_Obj *
TypeCodeGenTcl::emitValueBox (const CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ctc = tc->content_type();
  Tcl_Obj *o[3], *res;

  o[0] = Tcl_NewStringObj ("valuebox", 8);
  o[1] = Tcl_NewStringObj ((char *) tc->id(), -1);
  o[2] = emit (ctc);

  res = Tcl_NewListObj (3, o);
  return res;
}

/*
 * ----------------------------------------------------------------------
 * Scan a TypeCode
 * ----------------------------------------------------------------------
 */

TypeCodeScanTcl::TypeCodeScanTcl (Tcl_Interp *_i)
  : interp (_i)
{
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scan (Tcl_Obj * data)
{
  CORBA::TypeCode_ptr res = CORBA::TypeCode::_nil();
  const char * str;

  str = Tcl_GetStringFromObj (data, NULL);

  if (strcmp (str, "null") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_null);
  }
  else if (strcmp (str, "void") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_void);
  }
  else if (strcmp (str, "short") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_short);
  }
  else if (strcmp (str, "long") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_long);
  }
  else if (strcmp (str, "unsigned short") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_ushort);
  }
  else if (strcmp (str, "unsigned long") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_ulong);
  }
  else if (strcmp (str, "float") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_float);
  }
  else if (strcmp (str, "double") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_double);
  }
  else if (strcmp (str, "boolean") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_boolean);
  }
  else if (strcmp (str, "char") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_char);
  }
  else if (strcmp (str, "octet") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_octet);
  }
  else if (strcmp (str, "any") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_any);
  }
  else if (strcmp (str, "TypeCode") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_TypeCode);
  }
  else if (strcmp (str, "Principal") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_Principal);
  }
  else if (strcmp (str, "long long") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_longlong);
  }
  else if (strcmp (str, "unsigned long long") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_ulonglong);
  }
  else if (strcmp (str, "long double") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_longdouble);
  }
  else if (strcmp (str, "wchar") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_wchar);
  }
  else if (strcmp (str, "string") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_string);
  }
  else if (strcmp (str, "wstring") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_wstring);
  }
  else if (strcmp (str, "Object") == 0) {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_Object);
  }
  else {
    /*
     * Constructed or complex type
     */

    Tcl_Obj *o1;
    int len;

    if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
	Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan typecode from \"",
			  Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }

    str = Tcl_GetStringFromObj (o1, NULL);

    if (strcmp (str, "Object") == 0) {
      res = scanObjref (data);
    }
    else if (strcmp (str, "string") == 0) {
      res = scanString (data);
    }
    else if (strcmp (str, "wstring") == 0) {
      res = scanWstring (data);
    }
    else if (strcmp (str, "sequence") == 0) {
      res = scanSequence (data);
    }
    else if (strcmp (str, "recursive") == 0) {
      res = scanRecursive (data);
    }
    else if (strcmp (str, "array") == 0) {
      res = scanArray (data);
    }
    else if (strcmp (str, "fixed") == 0) {
      res = scanFixed (data);
    }
    else if (strcmp (str, "struct") == 0) {
      res = scanStruct (data);
    }
    else if (strcmp (str, "union") == 0) {
      res = scanUnion (data);
    }
    else if (strcmp (str, "enum") == 0) {
      res = scanEnum (data);
    }
    else if (strcmp (str, "exception") == 0) {
      res = scanException (data);
    }
    else if (strcmp (str, "valuetype") == 0) {
      res = scanValue (data);
    }
    else if (strcmp (str, "valuebox") == 0) {
      res = scanValueBox (data);
    }
    else {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan typecode from \"",
			  Tcl_GetStringFromObj (data, NULL), "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }
  }

  if (CORBA::is_nil (res)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while packing TypeCode from \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (data, NULL));
      Tcl_AddErrorInfo (interp, "\"");
    }
  }

  return res;
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanObjref (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2;
  const char *str, *id;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id  = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "Object") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"object reference typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  return Combat::GlobalData->orb->create_interface_tc (id, "");
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanString (Tcl_Obj * data)
{
  CORBA::TypeCode_ptr res;
  Tcl_Obj *o1, *o2;
  const char *str, *sb;
  unsigned long bound;
  char *sbe;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (sb  = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "string") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"bounded string typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  bound = strtoul (sb, &sbe, 0);

  if (*sbe) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: expected bound for string ",
			"but got \"", Tcl_GetStringFromObj (o2, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  if (bound) {
    res = Combat::GlobalData->orb->create_string_tc (bound);
  }
  else {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_string);
  }
  
  return res;
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanWstring (Tcl_Obj * data)
{
  CORBA::TypeCode_ptr res;
  Tcl_Obj *o1, *o2;
  const char *str, *sb;
  unsigned long bound;
  char *sbe;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (sb  = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "wstring") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"bounded wstring typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  bound = strtoul (sb, &sbe, 0);

  if (*sbe) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: expected bound for wstring ",
			"but got \"", Tcl_GetStringFromObj (o2, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  if (bound) {
    res = Combat::GlobalData->orb->create_wstring_tc (bound);
  }
  else {
    res = CORBA::TypeCode::_duplicate (CORBA::_tc_wstring);
  }
  
  return res;
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanSequence (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3;
  const char *str, *sb;
  unsigned long bound;
  char *sbe;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK ||
      (len != 2 && len != 3) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "sequence") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"sequence typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  CORBA::TypeCode_var ctc = scan (o2);

  if (CORBA::is_nil (ctc)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while scanning sequence TypeCode");
    }
    return CORBA::TypeCode::_nil();
  }

  if (len == 3) {
    if (Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan ",
			  "bounded sequence typecode from \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }

    sb = Tcl_GetStringFromObj (o3, NULL);
    bound = strtoul (sb, &sbe, 0);

    if (*sbe) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: expected bound for sequence ",
			  "but got \"", Tcl_GetStringFromObj (o3, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }
  }
  else {
    bound = 0;
  }

  return Combat::GlobalData->orb->create_sequence_tc (bound, ctc);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanRecursive (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2;
  const char *str, *id;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "recursive") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"recursive typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  return Combat::GlobalData->orb->create_recursive_tc (id);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanArray (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3;
  const char *str, *sb;
  unsigned long bound;
  char *sbe;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 3) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (sb  = Tcl_GetStringFromObj (o3, NULL)) == NULL ||
      strcmp (str, "array") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"array typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  CORBA::TypeCode_var ctc = scan (o2);

  if (CORBA::is_nil (ctc)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while scanning array TypeCode");
    }
    return CORBA::TypeCode::_nil();
  }

  bound = strtoul (sb, &sbe, 0);

  if (*sbe) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: expected bound for array ",
			"but got \"", Tcl_GetStringFromObj (o3, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  return Combat::GlobalData->orb->create_array_tc (bound, ctc);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanFixed (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3;
  CORBA::UShort digits;
  CORBA::Short scale;
  const char * str;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 3) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "fixed") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"fixed typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::Any * a1 = Combat::GetAnyFromObj (interp, NULL, o2,
					   CORBA::_tc_ushort);
  CORBA::Any * a2 = Combat::GetAnyFromObj (interp, NULL, o3,
					   CORBA::_tc_short);
  
  if (!a1 || !a2 || !(*a1 >>= digits) || !(*a2 >>= scale)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while scanning fixed TypeCode");
    }
    delete a1;
    delete a2;
    return CORBA::TypeCode::_nil();
  }

  delete a1;
  delete a2;

  return Combat::GlobalData->orb->create_fixed_tc (digits, scale);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanStruct (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3;
  const char *str, *id;
  int nmems, len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 3) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjLength (NULL, o3, &nmems) != TCL_OK || ((nmems % 2) != 0) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "struct") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"struct typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::StructMemberSeq sms;
  sms.length (nmems/2);

  for (int i=0; i<nmems/2; i++) {
    Tcl_Obj *mno, *mto;

    if (Tcl_ListObjIndex (NULL, o3, 2*i,   &mno) != TCL_OK ||
	Tcl_ListObjIndex (NULL, o3, 2*i+1, &mto) != TCL_OK) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan ",
			  "struct typecode from \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }
    
    const char * mname = Tcl_GetStringFromObj (mno, NULL);
    CORBA::TypeCode_var mtc = scan (mto);
    
    if (CORBA::is_nil (mtc)) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while scanning TypeCode of member \"");
	Tcl_AddErrorInfo (interp, (char *) mname);
	Tcl_AddErrorInfo (interp, "\"");
      }
      return CORBA::TypeCode::_nil();
    }

    sms[i].name     = mname;
    sms[i].type     = CORBA::TypeCode::_duplicate (mtc);
    sms[i].type_def = CORBA::IDLType::_nil();
  }
      
  return Combat::GlobalData->orb->create_struct_tc (id, "", sms);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanUnion (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3, *o4;
  const char *str, *id;
  int nmems, len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 4) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK ||
      Tcl_ListObjLength (NULL, o4, &nmems) != TCL_OK || ((nmems % 2) != 0) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "union") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"union typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::TypeCode_var dtype = scan (o3);

  if (CORBA::is_nil (dtype)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while scanning union discriminator");
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::UnionMemberSeq ums;
  ums.length (nmems/2);

  for (int i=0; i<nmems/2; i++) {
    Tcl_Obj *mlab, *mtype;
    const char *lname;
    CORBA::Any label;

    if (Tcl_ListObjIndex (NULL, o4, 2*i,   &mlab)  != TCL_OK ||
	Tcl_ListObjIndex (NULL, o4, 2*i+1, &mtype) != TCL_OK) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan ",
			  "union typecode from \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }
    
    lname = Tcl_GetStringFromObj (mlab, NULL);
    
    if (strcmp (lname, "(default)") == 0) {
      DynamicAny::DynAny_var da = Combat::GlobalData->daf->create_dyn_any_from_type_code (CORBA::_tc_octet);
      da->insert_octet ('\0');
      CORBA::Any_var val = da->to_any();
      label = val.in();
    }
    else {
      CORBA::Any * thelab = Combat::GetAnyFromObj (interp, NULL,
						   mlab, dtype);

      if (!thelab) {
	if (interp) {
	  Tcl_AddErrorInfo (interp, "\n  while scanning union label");
	}
	return CORBA::TypeCode::_nil();
      }
      label = *thelab;
      delete thelab;
    }
    
    CORBA::TypeCode_var mtc = scan (mtype);

    if (CORBA::is_nil (mtc)) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while scanning TypeCode of member");
      }
      return CORBA::TypeCode::_nil();
    }

    ums[i].name     = (const char *) "";
    ums[i].label    = label;
    ums[i].type     = CORBA::TypeCode::_duplicate (mtc);
    ums[i].type_def = CORBA::IDLType::_nil();
  }
      
  return Combat::GlobalData->orb->create_union_tc (id, "", dtype, ums);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanEnum (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2;
  const char *str;
  int nmems, len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 2) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjLength (NULL, o2, &nmems) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      strcmp (str, "enum") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"enum typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::EnumMemberSeq ems;
  ems.length (nmems);

  for (int i=0; i<nmems; i++) {
    Tcl_Obj *mobj;
    int count;

    if (Tcl_ListObjIndex (NULL, o2, i, &mobj) != TCL_OK ||
	Tcl_ListObjLength (NULL, mobj, &count) != TCL_OK ||
	count != 1) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan ",
			  "enum typecode from \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }

    ems[i] = (const char *) Tcl_GetStringFromObj (mobj, NULL);
  }

  return Combat::GlobalData->orb->create_enum_tc ("", "", ems);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanException (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3;
  const char *str, *id;
  int nmems, len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 3) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjLength (NULL, o3, &nmems) != TCL_OK || ((nmems % 2) != 0) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "exception") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"exception typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::StructMemberSeq sms;
  sms.length (nmems/2);

  for (int i=0; i<nmems/2; i++) {
    Tcl_Obj *mno, *mto;

    if (Tcl_ListObjIndex (NULL, o3, 2*i,   &mno) != TCL_OK ||
	Tcl_ListObjIndex (NULL, o3, 2*i+1, &mto) != TCL_OK) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan ",
			  "struct typecode from \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }
    
    const char * mname = Tcl_GetStringFromObj (mno, NULL);
    CORBA::TypeCode_var mtc = scan (mto);
    
    if (CORBA::is_nil (mtc)) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while scanning TypeCode of member \"");
	Tcl_AddErrorInfo (interp, (char *) mname);
	Tcl_AddErrorInfo (interp, "\"");
      }
      return CORBA::TypeCode::_nil();
    }

    sms[i].name     = mname;
    sms[i].type     = CORBA::TypeCode::_duplicate (mtc);
    sms[i].type_def = CORBA::IDLType::_nil();
  }
      
  return Combat::GlobalData->orb->create_exception_tc (id, "", sms);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanValue (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3, *o4, *o5;
  const char *str, *id, *modi, *bs;
  int nmems, len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 4) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 3, &o4) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 4, &o5) != TCL_OK ||
      Tcl_ListObjLength (NULL, o3, &nmems) != TCL_OK || ((nmems % 3) != 0) ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      (bs = Tcl_GetStringFromObj (o4, NULL)) == NULL ||
      (modi = Tcl_GetStringFromObj (o5, NULL)) == NULL ||
      strcmp (str, "valuetype") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"valuetype typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }

  CORBA::ValueMemberSeq vms;
  vms.length (nmems/3);

  for (int i=0; i<nmems/3; i++) {
    Tcl_Obj *mvo, *mno, *mto;
    const char *mvs;

    if (Tcl_ListObjIndex (NULL, o3, 3*i,   &mvo) != TCL_OK ||
	Tcl_ListObjIndex (NULL, o3, 3*i+1, &mno) != TCL_OK ||
	Tcl_ListObjIndex (NULL, o3, 3*i+2, &mto) != TCL_OK ||
	(mvs = Tcl_GetStringFromObj (mvo, NULL)) == NULL) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: could not scan ",
			  "valuetype typecode from \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\"", NULL);
      }
      return CORBA::TypeCode::_nil();
    }
    
    const char * mname = Tcl_GetStringFromObj (mno, NULL);
    CORBA::TypeCode_var mtc = scan (mto);
    
    if (CORBA::is_nil (mtc)) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while scanning TypeCode of member \"");
	Tcl_AddErrorInfo (interp, (char *) mname);
	Tcl_AddErrorInfo (interp, "\"");
      }
      return CORBA::TypeCode::_nil();
    }

    vms[i].name     = mname;
    vms[i].type     = CORBA::TypeCode::_duplicate (mtc);
    vms[i].type_def = CORBA::IDLType::_nil();

    if (strcmp (mvs, "private") == 0) {
      vms[i].access = CORBA::PRIVATE_MEMBER;
    }
    else {
      vms[i].access = CORBA::PUBLIC_MEMBER;
    }
  }

  CORBA::TypeCode_var base = CORBA::TypeCode::_nil ();

  if (strcmp (bs, "0") != 0) {
    base = scan (o4);

    if (CORBA::is_nil (base)) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while scanning base TypeCode");
      }
      return CORBA::TypeCode::_nil();
    }
  }

  CORBA::ValueModifier vm;

  if (strcmp (modi, "") == 0) {
    vm = CORBA::VM_NONE;
  }
  else if (strcmp (modi, "custom") == 0) {
    vm = CORBA::VM_CUSTOM;
  }
  else if (strcmp (modi, "abstract") == 0) {
    vm = CORBA::VM_ABSTRACT;
  }
  else if (strcmp (modi, "truncatable") == 0) {
    vm = CORBA::VM_TRUNCATABLE;
  }
  else {
    vm = CORBA::VM_NONE;
  }
  
  return Combat::GlobalData->orb->create_value_tc (id, "", vm, base, vms);
}

CORBA::TypeCode_ptr
TypeCodeScanTcl::scanValueBox (Tcl_Obj * data)
{
  Tcl_Obj *o1, *o2, *o3;
  const char *str, *id;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || (len != 2) ||
      Tcl_ListObjIndex (NULL, data, 0, &o1) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &o2) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 2, &o3) != TCL_OK ||
      (str = Tcl_GetStringFromObj (o1, NULL)) == NULL ||
      (id = Tcl_GetStringFromObj (o2, NULL)) == NULL ||
      strcmp (str, "valuebox") != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: could not scan ",
			"valuebox typecode from \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return CORBA::TypeCode::_nil();
  }
  
  CORBA::TypeCode_var ctc = scan (o2);

  if (CORBA::is_nil (ctc)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while scanning valuebox TypeCode");
    }
    return CORBA::TypeCode::_nil();
  }

  return Combat::GlobalData->orb->create_value_box_tc (id, "", ctc);
}

