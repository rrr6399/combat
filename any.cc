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
 * Handling of Any
 * ----------------------------------------------------------------------
 */

#include "combat.h"
#include <vector>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

char * combat_any_id = "$Id$";

/*
 * Extractor and Packer classes
 */

struct TclAnyData;

class Combat_Extractor {
public:
  Combat_Extractor (Tcl_Interp *, Combat::Context *, bool recurse = false);
  Tcl_Obj * Extract (DynamicAny::DynAny_ptr);

private:
  Tcl_Obj * Extract     (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Short    (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Long     (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_UShort   (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_ULong    (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_LongLong (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_ULongLong(DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Float    (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Double   (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_LongDouble(DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Boolean  (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Char     (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Octet    (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_WChar    (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_String   (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_WString  (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_TypeCode (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Any      (DynamicAny::DynAny_ptr);
  Tcl_Obj * ex_Objref   (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Fixed    (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Alias    (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Struct   (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Except   (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Sequence (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Array    (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Enum     (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Union    (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_Value    (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);
  Tcl_Obj * ex_ValueBox (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr);

  Tcl_Interp * interp;
  Combat::Context * ctx;
  bool recurse;
};

class Combat_Packer {
public:
  Combat_Packer (Tcl_Interp *, Combat::Context *);
  DynamicAny::DynAny_ptr Pack (Tcl_Obj *, const CORBA::TypeCode_ptr);

private:
  bool Pack (Tcl_Obj *, const CORBA::TypeCode_ptr,
	     DynamicAny::DynAny_ptr);

  bool pack_Short    (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Long     (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_UShort   (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_ULong    (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_LongLong (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_ULongLong(Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Float    (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Double   (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_LongDouble(Tcl_Obj *,DynamicAny::DynAny_ptr);
  bool pack_Boolean  (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Char     (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Octet    (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_WChar    (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_TypeCode (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Any      (Tcl_Obj *, DynamicAny::DynAny_ptr);
  bool pack_Objref   (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Fixed    (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_String   (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_WString  (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Alias    (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Struct   (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr,
		      const char * = NULL);
  bool pack_Except   (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Sequence (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Array    (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Enum     (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Union    (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_Value    (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);
  bool pack_ValueBox (Tcl_Obj *, const CORBA::TypeCode_ptr,
		      DynamicAny::DynAny_ptr);

  Tcl_Interp * interp;
  Combat::Context * ctx;
};

/*
 * ----------------------------------------------------------------------
 * Registration of CORBA::Any as a Tcl_Obj type
 * ----------------------------------------------------------------------
 */

struct TclAnyData {
  TclAnyData (Tcl_Interp *, Combat::Context *, DynamicAny::DynAny_ptr);
  TclAnyData (const TclAnyData &);
  ~TclAnyData ();

  Tcl_Interp * interp;
  Combat::Context * ctx;
  DynamicAny::DynAny_var any;
  Tcl_Obj * unrolled;
};

TclAnyData::TclAnyData (Tcl_Interp * _i, Combat::Context * _c,
			DynamicAny::DynAny_ptr _a)
{
  interp = _i;
  ctx = _c;
  any = _a;
  unrolled = NULL;
}

TclAnyData::TclAnyData (const TclAnyData & other)
{
  interp = other.interp;
  ctx = other.ctx;
  if (!CORBA::is_nil (other.any.in())) {
    any = other.any->copy();
    assert (!CORBA::is_nil (any));
  }
  else {
    any = DynamicAny::DynAny::_nil ();
  }
  unrolled = NULL;
}

TclAnyData::~TclAnyData ()
{
  if (!CORBA::is_nil (any)) {
    any->destroy ();
  }
  if (unrolled) {
    Tcl_DecrRefCount (unrolled);
  }
}

extern "C" {

/*
 * We install this function as TclList's setFromAnyProc and therefore
 * intercept any conversions to list.
 *
 * We do this to optimize access to our Any type. Conversion to list would
 * by default happen by unrolling the complete type into its string rep.
 * Instead, we extract complext types just one level deep to accomplish
 * the desired conversion to a list object, but its components remain
 * Any objects.
 */

int
Combat_ListFromAny (Tcl_Interp * interp, Tcl_Obj * obj)
{
  /*
   * Only care for conversions from the Any type. For everyhting
   * else, call the original TclList's setFromAnyProc.
   */

  if (obj->typePtr != &Combat::AnyType) {
    return Combat::OldListType.setFromAnyProc (interp, obj);
  }

  TclAnyData * objInf = (TclAnyData *) obj->internalRep.otherValuePtr;
  Tcl_Obj * res;

  /*
   * Unroll the Any one level deep
   */

  if (objInf->unrolled) {
    res = objInf->unrolled;
    objInf->unrolled = NULL;
  }
  else {
    assert (!CORBA::is_nil (objInf->any));
    Combat_Extractor ex (objInf->interp, objInf->ctx);
    res = ex.Extract (objInf->any);
  }
  assert (res->refCount == 0);

  /*
   * Free old (Any) representation
   */

  if (obj->typePtr != NULL && obj->typePtr->freeIntRepProc != NULL) {
    obj->typePtr->freeIntRepProc (obj);
  }

  /*
   * If res is of list type, we can simply steal its internal rep. But
   * it can also be a primitive type, or empty. In that case, call the
   * list conversion routine before stealing.
   */

  if (res->typePtr != Combat::ListTypePtr) {
    Tcl_ConvertToType (interp, res, Combat::ListTypePtr);
  }

  int refCount = obj->refCount;
  memcpy (obj, res, sizeof (Tcl_Obj));
  obj->refCount = refCount;

  res->typePtr = NULL;
  res->bytes   = NULL;
  Tcl_DecrRefCount (res);

  return TCL_OK;
}

/*
 * Any data type implementation
 */

static int
TclAny_SetFromAny (Tcl_Interp * interp, Tcl_Obj * obj)
{
  if (obj->typePtr == &Combat::AnyType) {
    return TCL_OK;
  }

  /*
   * Cannot convert a string directly to an any; we need a
   * TypeCode -- this operation must fail.
   */

  if (interp) {
    Tcl_SetResult (interp, "error: need typecode information", TCL_STATIC);
  }

  return TCL_ERROR;
}

static void
TclAny_UpdateString (Tcl_Obj * obj)
{
  TclAnyData * objInf = (TclAnyData *) obj->internalRep.otherValuePtr;
  assert (obj->typePtr == &Combat::AnyType);

  /*
   * Fully unroll the Any
   */

  if (!objInf->unrolled) {
    assert (!CORBA::is_nil (objInf->any));
    Combat_Extractor ex (objInf->interp, objInf->ctx, true);
    objInf->unrolled = ex.Extract (objInf->any);
  }

  /*
   * Force object to update string rep. This causes recursive calls.
   */

  if (objInf->unrolled->bytes == NULL) {
    objInf->unrolled->typePtr->updateStringProc (objInf->unrolled);
  }

  /*
   * If we're being used as a string, we probably don't need the Any data
   * anymore, so release the DynAny
   */

  if (!CORBA::is_nil (objInf->any)) {
    objInf->any->destroy ();
    objInf->any = DynamicAny::DynAny::_nil ();
  }

  /*
   * Steal the string rep
   */

  obj->bytes = objInf->unrolled->bytes;
  obj->length = objInf->unrolled->length;
  objInf->unrolled->bytes = NULL;
}

static void
TclAny_DupInternal (Tcl_Obj * src, Tcl_Obj * dup)
{
  assert (src->typePtr == &Combat::AnyType);

  TclAnyData * srcInf = (TclAnyData *) src->internalRep.otherValuePtr;
  TclAnyData * dupInf = new TclAnyData (*srcInf);

  assert (dupInf != NULL);

  dup->typePtr = src->typePtr;
  dup->internalRep.otherValuePtr = (VOID *) (void *) dupInf;
}

/*
 * In UpdateString, we morph into a new representation. Now if some code
 * stores the typePtr, calls updateStringProc and then releases the obj
 * using the stored typePtr, freeIntRepProc may be called here wrongly.
 * (Actually, this is just what Tcl's list type does.)
 * If this happens, just call the obj's new freeIntRepProc.
 */

static void
TclAny_FreeInternal (Tcl_Obj * obj)
{
  assert (obj->typePtr == &Combat::AnyType);
  TclAnyData * objInf = (TclAnyData *) obj->internalRep.otherValuePtr;
  delete objInf;
}

}

#ifdef HAVE_NAMESPACE
namespace Combat {
  Tcl_ObjType AnyType = {
    "CORBA::Any",
    TclAny_FreeInternal,
    TclAny_DupInternal,
    TclAny_UpdateString,
    TclAny_SetFromAny
  };
};
#else
Tcl_ObjType Combat::AnyType = {
  "CORBA::Any",
  TclAny_FreeInternal,
  TclAny_DupInternal,
  TclAny_UpdateString,
  TclAny_SetFromAny
};
#endif

/*
 * Create a new Any object. The Any is not consumed
 *
 * If the any contains any object references, then don't create a
 * CORBA::Any object but a Tcl list. Else, if a reference is extracted
 * from the list, CORBA::Any might be converted to a list via the string
 * representation, loosing our CmdType information.
 */

static bool
ContainsAnyObjectReference (CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ctc;
  static std::vector<void*> recursion;

  switch (tc->kind()) {
  case CORBA::tk_objref:
    return true;
  case CORBA::tk_struct:
  case CORBA::tk_except:
  case CORBA::tk_union:
  case CORBA::tk_value:
    {
      for (CORBA::ULong idx=0; idx<recursion.size(); idx++) {
	if ((void *) tc == recursion[idx]) {
	  return false;
	}
      }

      recursion.push_back ((void *) tc);

      CORBA::ULong len = tc->member_count();
      for (CORBA::ULong i=0; i<len; i++) {
	ctc = tc->member_type (i);

	if (ContainsAnyObjectReference (ctc.in())) {
	  recursion.pop_back ();
	  return true;
	}
	else if (ctc->kind() == CORBA::tk_objref) {
	  recursion.pop_back ();
	  return true;
	}
      }
      recursion.pop_back ();
    }
    break;
  case CORBA::tk_sequence:
  case CORBA::tk_array:
  case CORBA::tk_value_box:
    ctc = tc->content_type ();
    return ContainsAnyObjectReference (ctc.in());
  case CORBA::tk_alias:
    ctc = tc->content_type ();
    return ContainsAnyObjectReference (ctc.in());
  }
  return false;
}

Tcl_Obj *
Combat::NewAnyObj (Tcl_Interp * interp, Context * ctx,
		   const CORBA::Any & any)
{
  CORBA::TypeCode_var tc = any.type ();

  /*
   * Shortcut for sequence<octet> to avoid all the conversions between
   * Any and DynAny. For the moment, this only works with MICO, which
   * provides Any insertion and extraction for CORBA::OctetSeq
   */

#if !defined(COMBAT_USE_ORBACUS) && !defined(COMBAT_USE_ORBIX)
  CORBA::TypeCode_var uatc = CORBA::TypeCode::_duplicate (tc);
  while (uatc->kind() == CORBA::tk_alias)
    uatc = uatc->content_type ();

  if (uatc->kind() == CORBA::tk_sequence && uatc->length() == 0) {
    CORBA::TypeCode_var ctc = uatc->content_type ();
    while (ctc->kind() == CORBA::tk_alias)
      ctc = ctc->content_type ();

    if (ctc->kind() == CORBA::tk_octet || ctc->kind() == CORBA::tk_char) {
      CORBA::OctetSeq os;
      CORBA::CharSeq cs;
      CORBA::Octet * buf;
      CORBA::ULong len;
      CORBA::Boolean r;

      switch (ctc->kind()) {
      case CORBA::tk_octet:
	{
	  r = (any >>= os);
	  assert (r);
	  len = os.length ();
	  buf = len ? os.get_buffer() : NULL;
	}
	break;
      case CORBA::tk_char:
	{
	  r = (any >>= cs);
	  assert (r);
	  len = cs.length ();
	  buf = (CORBA::Octet *) (len ? cs.get_buffer() : NULL);
	}
	break;
      }
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
      return Tcl_NewStringObj ((char *) buf, len);
#else
      return Tcl_NewByteArrayObj ((unsigned char *) buf, len);
#endif
    }
  }
#endif

  /*
   * Create a CORBA::Any object
   */

  DynamicAny::DynAny_ptr dynany =
    Combat::GlobalData->daf->create_dyn_any (any);

  if (ContainsAnyObjectReference (tc.in())) {
    Combat_Extractor ex (interp, ctx);
    Tcl_Obj * res = ex.Extract (dynany);
    dynany->destroy ();
    CORBA::release (dynany);
    return res;
  }

  Tcl_Obj * obj = Tcl_NewObj ();
  TclAnyData * objInf = new TclAnyData (interp, ctx, dynany);

  assert (obj && objInf);

  Tcl_InvalidateStringRep (obj);
  
  obj->typePtr = &AnyType;
  obj->internalRep.otherValuePtr = (VOID *) (void *) objInf;

  return obj;
}

Tcl_Obj *
Combat::NewAnyObj (const CORBA::Any & any)
{
  return NewAnyObj (NULL, NULL, any);
}

/*
 * Get an Any value from a Tcl_Obj. Either use our tclAnyType internal
 * representation, or try to convert the string to an any
 */

CORBA::Any *
Combat::GetAnyFromObj (Tcl_Interp * interp,
		       Context * ctx, Tcl_Obj * data,
		       const CORBA::TypeCode_ptr tc)
{
  TclAnyData * objInf;

  /*
   * If data is already an any object, and the typecodes match,
   * use the internal rep.
   *
   * If the typecode doesn't match, try to convert the object to
   * the requested type
   */

  if (data->typePtr == &AnyType) {
    objInf = (TclAnyData *) data->internalRep.otherValuePtr;
    if (!CORBA::is_nil (objInf->any)) {
      CORBA::TypeCode_var objtc = objInf->any->type ();

      if (tc->equal (objtc)) {
	return objInf->any->to_any ();
      }
    }
  }

  /*
   * Shortcut for sequence<octet>
   */

#if !defined(COMBAT_USE_ORBACUS) && !defined(COMBAT_USE_ORBIX)
  CORBA::TypeCode_var uatc = CORBA::TypeCode::_duplicate (tc);
  while (uatc->kind() == CORBA::tk_alias)
    uatc = uatc->content_type ();

  if (uatc->kind() == CORBA::tk_sequence && uatc->length() == 0) {
    CORBA::TypeCode_var ctc = uatc->content_type ();
    while (ctc->kind() == CORBA::tk_alias)
      ctc = ctc->content_type ();

    if (ctc->kind() == CORBA::tk_octet || ctc->kind() == CORBA::tk_char) {
      CORBA::Octet * buf;
      int llen;
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
      buf = (CORBA::Octet *) Tcl_GetStringFromObj (data, &llen);
#else
      buf = (CORBA::Octet *) Tcl_GetByteArrayFromObj (data, &llen);
#endif
      CORBA::Any * any = new CORBA::Any;
      switch (ctc->kind()) {
      case CORBA::tk_octet:
	{
	  CORBA::OctetSeq os (llen, llen, buf);
	  *any <<= os;
	}
	break;
      case CORBA::tk_char:
	{
	  CORBA::CharSeq cs (llen, llen, (CORBA::Char *) buf);
	  *any <<= cs;
	}
      }
      return any;
    }
  }
#endif

  /*
   * Try to convert it to an any value
   */

  Combat_Packer p (interp, ctx);
  DynamicAny::DynAny_ptr val = p.Pack (data, tc);

  if (CORBA::is_nil (val)) {
    return NULL;
  }

  /*
   * Never convert handles
   */

  if (data->typePtr == CmdTypePtr) {
    CORBA::Any * res = val->to_any ();
    val->destroy ();
    CORBA::release (val);
    return res;
  }

  if (data->typePtr != NULL && data->typePtr->freeIntRepProc != NULL) {
    data->typePtr->freeIntRepProc (data);
  }

  /*
   * Okay, so change the internal representation
   */

  objInf = new TclAnyData (interp, ctx, val);
  data->typePtr = &AnyType;
  data->internalRep.otherValuePtr = (VOID *) (void *) objInf;

  /*
   * Return any data
   */

  return val->to_any ();
}

/*
 * ----------------------------------------------------------------------
 * Extract a potentially complex type to a Tcl object
 * ----------------------------------------------------------------------
 */

Combat_Extractor::Combat_Extractor (Tcl_Interp * _i, Combat::Context * _c,
				    bool _r)
{
  interp = _i;
  ctx = _c;
  recurse = _r;
}

Tcl_Obj *
Combat_Extractor::ex_Short (DynamicAny::DynAny_ptr any)
{
  CORBA::Short val = any->get_short ();
  return Tcl_NewLongObj (val);
}

Tcl_Obj *
Combat_Extractor::ex_Long (DynamicAny::DynAny_ptr any)
{
  CORBA::Long val = any->get_long ();
  return Tcl_NewLongObj (val);
}

/*
 * unsigned values might exceed Tcl's long, so handle them as strings
 */


Tcl_Obj *
Combat_Extractor::ex_UShort (DynamicAny::DynAny_ptr any)
{
  CORBA::UShort val = any->get_ushort ();
  char tmp[64];
  sprintf (tmp, "%lu", (unsigned long) val);
  return Tcl_NewStringObj (tmp, -1);
}

Tcl_Obj *
Combat_Extractor::ex_ULong (DynamicAny::DynAny_ptr any)
{
  CORBA::ULong val = any->get_ulong ();
  char tmp[64];
  sprintf (tmp, "%lu", (unsigned long) val);
  return Tcl_NewStringObj (tmp, -1);
}

/*
 * how do we represent long long's?
 */

Tcl_Obj *
Combat_Extractor::ex_LongLong (DynamicAny::DynAny_ptr any)
{
  CORBA::LongLong val = any->get_longlong ();
  char tmp[64];
  sprintf (tmp, "%Ld", (long long) val);
  return Tcl_NewStringObj (tmp, -1);
}

Tcl_Obj *
Combat_Extractor::ex_ULongLong (DynamicAny::DynAny_ptr any)
{
  CORBA::ULongLong val = any->get_ulonglong ();
  char tmp[64];
  sprintf (tmp, "%Lu", (unsigned long long) val);
  return Tcl_NewStringObj (tmp, -1);
}

Tcl_Obj *
Combat_Extractor::ex_Float (DynamicAny::DynAny_ptr any)
{
  CORBA::Float val = any->get_float ();
  return Tcl_NewDoubleObj (val);
}

Tcl_Obj *
Combat_Extractor::ex_Double (DynamicAny::DynAny_ptr any)
{
  CORBA::Double val = any->get_double ();
  return Tcl_NewDoubleObj (val);
}

Tcl_Obj *
Combat_Extractor::ex_LongDouble (DynamicAny::DynAny_ptr any)
{
  CORBA::LongDouble val = any->get_longdouble ();
  return Tcl_NewDoubleObj (val);
}

Tcl_Obj *
Combat_Extractor::ex_Boolean (DynamicAny::DynAny_ptr any)
{
  CORBA::Boolean val = any->get_boolean ();
  return Tcl_NewBooleanObj (val ? 1 : 0);
}

Tcl_Obj *
Combat_Extractor::ex_Char (DynamicAny::DynAny_ptr any)
{
  CORBA::Char val = any->get_char ();
  return Tcl_NewStringObj ((char *) &val, 1);
}

Tcl_Obj *
Combat_Extractor::ex_Octet (DynamicAny::DynAny_ptr any)
{
  CORBA::Octet val = any->get_octet ();
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  return Tcl_NewStringObj ((char *) &val, 1);
#else
  return Tcl_NewByteArrayObj ((unsigned char *) &val, 1);
#endif
}

Tcl_Obj *
Combat_Extractor::ex_WChar (DynamicAny::DynAny_ptr any)
{
  CORBA::WChar val = any->get_wchar ();
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  return Tcl_NewStringObj ((char *) &val, sizeof (CORBA::WChar));
#else
  char buf[TCL_UTF_MAX];
  int cnt = Tcl_UniCharToUtf ((int) val, buf);
  return Tcl_NewStringObj (buf, cnt);
#endif
}

Tcl_Obj *
Combat_Extractor::ex_String (DynamicAny::DynAny_ptr any)
{
  CORBA::String_var val = any->get_string ();
  return Tcl_NewStringObj ((char *) val.in(), -1);
}

/*
 * Unfortunately, Tcl and Mico do not necessarily agree on the size of WChar
 */

Tcl_Obj *
Combat_Extractor::ex_WString (DynamicAny::DynAny_ptr any)
{
  CORBA::WString_var val = any->get_wstring ();
  CORBA::ULong len;
  Tcl_Obj * res;

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  for (len=0; val[len]; len++);
  res = Tcl_NewStringObj ((char *) val.in(), (len+1)*sizeof(CORBA::WChar));
#else
  Tcl_UniChar * buf;
  Tcl_DString ds;

  if (sizeof (CORBA::WChar) == sizeof (Tcl_UniChar)) {
    buf = (Tcl_UniChar *) val.in();
    len = Tcl_UniCharLen (buf);
  }
  else {
    for (len=0; val[len]; len++);
    buf = new Tcl_UniChar[len+1];

    for (CORBA::ULong i=0; i<len; i++) {
      buf[i] = (Tcl_UniChar) val[i];
    }

    buf[len] = 0;
  }

  Tcl_DStringInit (&ds);
  Tcl_UniCharToUtfDString (buf, len, &ds);
  res = Tcl_NewStringObj (Tcl_DStringValue (&ds),
			  Tcl_DStringLength (&ds));
  Tcl_DStringFree (&ds);

  if (sizeof (CORBA::WChar) != sizeof (Tcl_UniChar)) {
    delete [] buf;
  }
#endif

  return res;
}

Tcl_Obj *
Combat_Extractor::ex_TypeCode (DynamicAny::DynAny_ptr any)
{
  CORBA::TypeCode_var ctc = any->get_typecode ();
  return Combat::NewTypeCodeObj (ctc.in());
}

Tcl_Obj *
Combat_Extractor::ex_Any (DynamicAny::DynAny_ptr any)
{
  CORBA::Any_var ca = any->get_any ();
  CORBA::TypeCode_var ctc = ca->type ();
  Tcl_Obj *o[2];

  o[0] = Combat::NewTypeCodeObj (ctc.in());
  if (recurse) {
    DynamicAny::DynAny_var na = Combat::GlobalData->daf->create_dyn_any (*ca);
    o[1] = Extract (na.in());
    na->destroy ();
  }
  else {
    o[1] = Combat::NewAnyObj (interp, ctx, ca.in());
  }

  return Tcl_NewListObj (2, o);
}

Tcl_Obj *
Combat_Extractor::ex_Objref (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * res;

  /*
   * Map nil references to `0', incarnate others into a handle
   */

  CORBA::Object_ptr obj = any->get_reference ();

  if (CORBA::is_nil (obj)) {
    res = Tcl_NewIntObj (0);
  }
  else {
    assert (ctx);
    if ((res = Combat::InstantiateObj (interp, ctx, obj)) == NULL) {
      CORBA::release (obj);
    }
    else {
      /*
       * Update type info to the type we've seen in the TypeCode
       */

      const char * objName = Tcl_GetStringFromObj (res, NULL);
      Combat::Context::ActiveObjTable::iterator ait =
	ctx->active.find (objName);
      assert (ait != ctx->active.end());
      (*ait).second->UpdateType (tc->id());
    }
  }

  return res;
}

Tcl_Obj *
Combat_Extractor::ex_Fixed (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynFixed_var fixed = DynamicAny::DynFixed::_narrow (any);
  assert (!CORBA::is_nil (fixed));
  CORBA::String_var val = fixed->get_value();
  return Tcl_NewStringObj ((char *) val.in(), -1);
}

Tcl_Obj *
Combat_Extractor::ex_Alias (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ctc = tc->content_type ();
  return Extract (any, ctc);
}

Tcl_Obj *
Combat_Extractor::ex_Struct (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  CORBA::ULong i, len = tc->member_count();
  Tcl_Obj *res;

  res = Tcl_NewObj ();

  for (i=0; i<len; i++) {
    Tcl_Obj * m[2];
    CORBA::TypeCode_var ntc = tc->member_type (i);
    const char * mname = tc->member_name (i);
    DynamicAny::DynAny_var member = any->current_component();
    m[0] = Tcl_NewStringObj ((char *) mname, -1);
    if (recurse) {
      m[1] = Extract (member.in());
    }
    else {
      CORBA::Any_var many = member->to_any();
      m[1] = Combat::NewAnyObj (interp, ctx, many.in());
    }
    Tcl_ListObjAppendElement (NULL, res, m[0]);
    Tcl_ListObjAppendElement (NULL, res, m[1]);
    any->next();
  }

  return res;
}

Tcl_Obj *
Combat_Extractor::ex_Except (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * r[2];
  r[0] = Tcl_NewStringObj ((char *) tc->id(), -1);
  r[1] = ex_Struct (any, tc);
  return Tcl_NewListObj (2, r);
}

Tcl_Obj *
Combat_Extractor::ex_Sequence (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynSequence_var ds = DynamicAny::DynSequence::_narrow (any);
  CORBA::TypeCode_var ctc = tc->content_type();
  Tcl_Obj *res;

  /*
   * Special handling for octet and char sequences, which are extracted
   * into a string (or ByteArray) rather than listed
   */

  CORBA::TypeCode_var uatc = CORBA::TypeCode::_duplicate (ctc);
  while (uatc->kind() == CORBA::tk_alias)
    uatc = uatc->content_type ();

  if (uatc->kind() == CORBA::tk_octet || uatc->kind() == CORBA::tk_char) {
    CORBA::OctetSeq os;
    CORBA::CharSeq cs;
    CORBA::Octet * buf;
    CORBA::ULong len;

    /*
     * There's no simple CORBA-compliant way of getting a lot of octets
     * at once from the DynAny if the sequence is bounded, as sequence
     * typecodes with a different length aren't equivalent, and av>>=os
     * would fail with BAD_TYPECODE. Bummer.
     * And ORBacus does not have an Any extraction operator for OctetSeq.
     */

#if defined(COMBAT_USE_MICO)
    if (tc->length()) {
#endif
      len = ds->get_length();
      os.length (len);
      buf = os.get_buffer ();
      switch (uatc->kind()) {
      case CORBA::tk_octet:
	{
	  for (CORBA::ULong idx=0; idx<len; idx++) {
	    buf[idx] = ds->get_octet ();
	    ds->next ();
	  }
	}
        break;
      case CORBA::tk_char:
	{
	  for (CORBA::ULong idx=0; idx<len; idx++) {
	    buf[idx] = ds->get_char ();
	    ds->next ();
	  }
	}
        break;
      default:
	assert (0);
      }
#if defined(COMBAT_USE_MICO)
    }
    else {
      CORBA::Any_var av = ds->to_any ();
      CORBA::Boolean r;
      switch (uatc->kind()) {
      case CORBA::tk_octet:
	{
	  r = (*av >>= os);
	  assert (r);
	  if ((len = os.length ()) > 0) {
	    buf = os.get_buffer ();
	  }
	  else {
	    buf = NULL;
	  }
	}
        break;
      case CORBA::tk_char:
	{
	  r = (*av >>= cs);
	  assert (r);
	  if ((len = cs.length ()) > 0) {
	    buf = (CORBA::Octet *) cs.get_buffer ();
	  }
	  else {
	    buf = NULL;
	  }
	}
        break;
      default:
	assert (0);
      }
    }
#endif
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
    return Tcl_NewStringObj ((char *) buf, len);
#else
    return Tcl_NewByteArrayObj ((unsigned char *) buf, len);
#endif
  }

  res = Tcl_NewObj ();
  
  for (CORBA::ULong i=ds->get_length(); i; i--) {
    DynamicAny::DynAny_var member = ds->current_component ();
    Tcl_Obj * ts;
    if (recurse) {
      ts = Extract (member.in());
    }
    else {
      CORBA::Any_var many = member->to_any();
      ts = Combat::NewAnyObj (interp, ctx, many.in());
    }
    Tcl_ListObjAppendElement (NULL, res, ts);
    ds->next();
  }

  return res;
}

Tcl_Obj *
Combat_Extractor::ex_Array (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var ctc = tc->content_type();
  Tcl_Obj *res;

  /*
   * Special handling for octet and char arrays, which are extracted
   * into a string (or ByteArray) rather than listed
   */

  CORBA::TypeCode_var uatc = CORBA::TypeCode::_duplicate (ctc);
  while (uatc->kind() == CORBA::tk_alias)
    uatc = uatc->content_type ();

  if (uatc->kind() == CORBA::tk_octet || uatc->kind() == CORBA::tk_char) {
    CORBA::ULong len = tc->length();
    CORBA::Octet * buf = new CORBA::Octet[len];
    switch (uatc->kind()) {
    case CORBA::tk_octet:
      {
	for (CORBA::ULong idx=0; idx<len; idx++) {
	  buf[idx] = any->get_octet ();
	  any->next ();
	}
      }
      break;
    case CORBA::tk_char:
      {
	for (CORBA::ULong idx=0; idx<len; idx++) {
	  buf[idx] = (CORBA::Octet) any->get_char ();
	  any->next ();
	}
      }
      break;
    default:
      assert (0);
    }
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
    res = Tcl_NewStringObj ((char *) buf, len);
#else
    res = Tcl_NewByteArrayObj ((unsigned char *) buf, len);
#endif
    delete [] buf;
    return res;
  }

  res = Tcl_NewObj ();

  for (CORBA::ULong i=tc->length(); i; i--) {
    DynamicAny::DynAny_var member = any->current_component ();
    Tcl_Obj * ts;
    if (recurse) {
      ts = Extract (member.in());
    }
    else {
      CORBA::Any_var many = member->to_any();
      ts = Combat::NewAnyObj (interp, ctx, many.in());
    }
    Tcl_ListObjAppendElement (NULL, res, ts);
    any->next();
  }

  return res;
}

Tcl_Obj *
Combat_Extractor::ex_Enum (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynEnum_var de = DynamicAny::DynEnum::_narrow (any);
  CORBA::String_var str = de->get_as_string ();
  return Tcl_NewStringObj ((char *) str.in(), -1);
}

Tcl_Obj *
Combat_Extractor::ex_Union (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynUnion_var du = DynamicAny::DynUnion::_narrow (any);
  Tcl_Obj *m[2], *res;

  DynamicAny::DynAny_var anydisc = du->get_discriminator ();
  if (recurse) {
    m[0] = Extract (anydisc.in());
  }
  else {
    CORBA::Any_var dany = anydisc->to_any();
    m[0] = Combat::NewAnyObj (interp, ctx, dany.in());
  }

  if (du->has_no_active_member()) {
    m[1] = Tcl_NewObj ();
  }
  else {
    DynamicAny::DynAny_var member = du->member ();
    if (recurse) {
      m[1] = Extract (member.in());
    }
    else {
      CORBA::Any_var many = member->to_any();
      m[1] = Combat::NewAnyObj (interp, ctx, many.in());
    }
  }

  res = Tcl_NewListObj (2, m);
  return res;
}

#if (!defined(COMBAT_USE_ORBACUS) || OB_INTEGER_VERSION >= 4010000) && \
	(!defined(COMBAT_USE_MICO) || MICO_BIN_VERSION >= 0x020306)

Tcl_Obj *
Combat_Extractor::ex_Value (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynValue_var dv = DynamicAny::DynValue::_narrow (any);

  /*
   * See if this valuetype is null
   */

  if (dv->is_null ()) {
    return Tcl_NewIntObj (0);
  }

  /*
   * Elements must be extracted from the innermost element outwards
   */

  std::vector<CORBA::TypeCode_var> bases;
  CORBA::TypeCode_var iter = CORBA::TypeCode::_duplicate (tc);

  while (!CORBA::is_nil (iter)) {
    bases.push_back (iter);
    iter = iter->concrete_base_type ();
  }

  Tcl_Obj * res = Tcl_NewObj ();

  while (bases.size() > 0) {
    iter = bases.back ();
    bases.pop_back ();

    CORBA::ULong len = iter->member_count();

    for (CORBA::ULong i=0; i<len; i++) {
      Tcl_Obj * m[2];
      CORBA::TypeCode_var ntc = iter->member_type (i);
      const char * mname = iter->member_name (i);
      DynamicAny::DynAny_var member = any->current_component();
      m[0] = Tcl_NewStringObj ((char *) mname, -1);
      if (recurse) {
	m[1] = Extract (member.in());
      }
      else {
	CORBA::Any_var many = member->to_any();
	m[1] = Combat::NewAnyObj (interp, ctx, many.in());
      }
      Tcl_ListObjAppendElement (NULL, res, m[0]);
      Tcl_ListObjAppendElement (NULL, res, m[1]);
      any->next();
    }
  }

  Tcl_Obj * tn = Tcl_NewStringObj ("_tc_", 4);
  Tcl_Obj * tv = Combat::NewTypeCodeObj (tc);

  Tcl_ListObjAppendElement (NULL, res, tn);
  Tcl_ListObjAppendElement (NULL, res, tv);

  return res;
}

Tcl_Obj *
Combat_Extractor::ex_ValueBox (DynamicAny::DynAny_ptr any,
			       CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynValueBox_var dv = DynamicAny::DynValueBox::_narrow (any);

  /*
   * See if this valuetype is null
   */

  if (dv->is_null ()) {
    return Tcl_NewIntObj (0);
  }

  Tcl_Obj * res;

  if (recurse) {
    DynamicAny::DynAny_var member = dv->get_boxed_value_as_dyn_any ();
    res = Extract (member.in());
  }
  else {
    CORBA::Any_var many = dv->get_boxed_value ();
    res = Combat::NewAnyObj (interp, ctx, many.in());
  }

  return res;
}

#else

Tcl_Obj *
Combat_Extractor::ex_Value (DynamicAny::DynAny_ptr, CORBA::TypeCode_ptr)
{
  assert (0);
  return Tcl_NewIntObj (0);
}

Tcl_Obj *
Combat_Extractor::ex_ValueBox (DynamicAny::DynAny_ptr,
			       CORBA::TypeCode_ptr)
{
  assert (0);
  return Tcl_NewIntObj (0);
}

#endif

/*
 * Extractor Main
 */

Tcl_Obj *
Combat_Extractor::Extract (DynamicAny::DynAny_ptr any)
{
  CORBA::TypeCode_var tc = any->type();
  return Extract (any, tc);
}

Tcl_Obj *
Combat_Extractor::Extract (DynamicAny::DynAny_ptr any, CORBA::TypeCode_ptr tc)
{
  Tcl_Obj * res = NULL;

#ifdef HAVE_EXCEPTIONS
  try {
#endif

  any->rewind();

  switch (tc->kind()) {

    /*
     * Primitive types
     */

  case CORBA::tk_null:
    res = Tcl_NewObj ();
    break;

  case CORBA::tk_void:
    res = Tcl_NewObj ();
    break;

  case CORBA::tk_short:
    res = ex_Short (any);
    break;

  case CORBA::tk_long:
    res = ex_Long (any);
    break;

  case CORBA::tk_ushort:
    res = ex_UShort (any);
    break;

  case CORBA::tk_ulong:
    res = ex_ULong (any);
    break;

  case CORBA::tk_float:
    res = ex_Float (any);
    break;

  case CORBA::tk_double:
    res = ex_Double (any);
    break;

  case CORBA::tk_boolean:
    res = ex_Boolean (any);
    break;

  case CORBA::tk_char:
    res = ex_Char (any);
    break;

  case CORBA::tk_octet:
    res = ex_Octet (any);
    break;

  case CORBA::tk_wchar:
    res = ex_WChar (any);
    break;

  case CORBA::tk_string:
    res = ex_String (any);
    break;

  case CORBA::tk_wstring:
    res = ex_WString (any);
    break;

  case CORBA::tk_longlong:
    res = ex_LongLong (any);
    break;

  case CORBA::tk_ulonglong:
    res = ex_ULongLong (any);
    break;

  case CORBA::tk_longdouble:
    res = ex_LongDouble (any);
    break;

  case CORBA::tk_TypeCode:
    res = ex_TypeCode (any);
    break;

  case CORBA::tk_Principal:
    assert (0);
    break;

  case CORBA::tk_any:
    res = ex_Any (any);
    break;

    /*
     * Complex types
     */

  case CORBA::tk_objref:
    res = ex_Objref (any, tc);
    break;

  case CORBA::tk_fixed:
    res = ex_Fixed (any, tc);
    break;

  case CORBA::tk_alias:
    res = ex_Alias (any, tc);
    break;

  case CORBA::tk_struct:
    res = ex_Struct (any, tc);
    break;

  case CORBA::tk_sequence:
    res = ex_Sequence (any, tc);
    break;

  case CORBA::tk_array:
    res = ex_Array (any, tc);
    break;

  case CORBA::tk_enum:
    res = ex_Enum (any, tc);
    break;

  case CORBA::tk_union:
    res = ex_Union (any, tc);
    break;

  case CORBA::tk_except:
    res = ex_Except (any, tc);
    break;

  case CORBA::tk_value:
    res = ex_Value (any, tc);
    break;

  case CORBA::tk_value_box:
    res = ex_ValueBox (any, tc);
    break;

  default:
    assert (0);
  }

#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    if (interp) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
    }
    return NULL;
  }
#endif

  assert (res);
  return res;
}

/*
 * ----------------------------------------------------------------------
 * Compose a potentially complex type from a Tcl object
 * ----------------------------------------------------------------------
 */

Combat_Packer::Combat_Packer (Tcl_Interp * _i, Combat::Context * _c)
{
  interp = _i;
  ctx = _c;
}

/*
 * Unfortunately, Tcl_GetLongFromObj is buggy in Tcl 8.0; it doesn't
 * complain if the value is too large for a long, but fits an unsigned
 * long.
 */

bool
Combat_Packer::pack_Short (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  CORBA::Short val;
  unsigned long tval;
  char *tmp, *ptr;
  int oops=0;

  tmp = Tcl_GetStringFromObj (data, NULL);
  while (*tmp == ' ') tmp++;

  errno = 0;

  if (*tmp == '-') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
    if ((CORBA::UShort) tval != tval) oops = 1;
    if ((val = -((CORBA::UShort) tval)) > 0) oops = 1;
  }
  else if (*tmp == '+') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
    if ((CORBA::UShort) tval != tval) oops = 1;
    if ((val = ((CORBA::UShort) tval)) < 0) oops = 1;
  }
  else {
    tval = strtoul (tmp, &ptr, 0);
    if ((CORBA::UShort) tval != tval) oops = 1;
    if ((val = ((CORBA::UShort) tval)) < 0) oops = 1;
  }

  if (*ptr == '.') {
    ptr++;
    while (*ptr == '0') ptr++;
  }

  while (*ptr == ' ') {
    ptr++;
  }

  if (*ptr) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"short\"", NULL);
    }
    return false;
  }

  if (oops || errno == ERANGE) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not fit \"short\"", NULL);
    }
    return false;
  }

  da->insert_short (val);
  return true;
}

bool
Combat_Packer::pack_Long (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  CORBA::Long val;
  unsigned long tval;
  char *tmp, *ptr;
  int oops=0;

  tmp = Tcl_GetStringFromObj (data, NULL);
  while (*tmp == ' ') tmp++;

  errno = 0;

  if (*tmp == '-') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
    if ((CORBA::ULong) tval != tval) oops = 1;
    if ((val = -((CORBA::ULong) tval)) > 0) oops = 1;
  }
  else if (*tmp == '+') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
    if ((CORBA::ULong) tval != tval) oops = 1;
    if ((val = ((CORBA::ULong) tval)) < 0) oops = 1;
  }
  else {
    tval = strtoul (tmp, &ptr, 0);
    if ((CORBA::ULong) tval != tval) oops = 1;
    if ((val = ((CORBA::ULong) tval)) < 0) oops = 1;
  }

  if (*ptr == '.') {
    ptr++;
    while (*ptr == '0') ptr++;
  }

  while (*ptr == ' ') {
    ptr++;
  }

  if (*ptr) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"long\"", NULL);
    }
    return false;
  }

  if (oops || errno == ERANGE) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not fit \"long\"", NULL);
    }
    return false;
  }

  da->insert_long (val);
  return true;
}

/*
 * unsigned values might exceed Tcl's long, so handle them as strings
 */

bool
Combat_Packer::pack_UShort (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  CORBA::UShort val;
  unsigned long tval;
  char *tmp, *ptr;
  int oops=0;

  errno = 0;

  tmp = Tcl_GetStringFromObj (data, NULL);
  while (*tmp == ' ') tmp++;

  if (*tmp == '-') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
    if (tval > 0) oops = 1;
  }
  else if (*tmp == '+') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
  }
  else {
    tval = strtoul (tmp, &ptr, 0);
  }

  if (*ptr == '.') {
    ptr++;
    while (*ptr == '0') ptr++;
  }

  while (*ptr == ' ') {
    ptr++;
  }

  if (*ptr) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"unsigned short\"", NULL);
    }
    return false;
  }
  if ((val = (CORBA::UShort) tval) != tval || errno == ERANGE || oops) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not fit \"unsigned short\"", NULL);
    }
    return false;
  }

  da->insert_ushort (val);
  return true;
}

bool
Combat_Packer::pack_ULong (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  CORBA::ULong val;
  unsigned long tval;
  char *tmp, *ptr;
  int oops=0;

  errno = 0;

  tmp = Tcl_GetStringFromObj (data, NULL);
  while (*tmp == ' ') tmp++;

  if (*tmp == '-') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
    if (tval > 0) oops = 1;
  }
  else if (*tmp == '+') {
    tmp++;
    tval = strtoul (tmp, &ptr, 0);
  }
  else {
    tval = strtoul (tmp, &ptr, 0);
  }

  if (*ptr == '.') {
    ptr++;
    while (*ptr == '0') ptr++;
  }

  while (*ptr == ' ') {
    ptr++;
  }

  if (*ptr) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"unsigned long\"", NULL);
    }
    return false;
  }
  if ((val = (CORBA::ULong) tval) != tval || errno == ERANGE || oops) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not fit \"unsigned long\"", NULL);
    }
    return false;
  }

  da->insert_ulong (val);
  return true;
}

/*
 * how do we represent long long's?
 */

bool
Combat_Packer::pack_LongLong (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  long long tval;
  char *tmp;

  tmp = Tcl_GetStringFromObj (data, NULL);

  if (sscanf (tmp, "%Ld", &tval) != 1) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not fit \"long long\"", NULL);
    }
    return false;
  }

  da->insert_longlong ((CORBA::LongLong) tval);
  return true;
}

bool
Combat_Packer::pack_ULongLong (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  unsigned long long tval;
  char *tmp;

  tmp = Tcl_GetStringFromObj (data, NULL);

  if (sscanf (tmp, "%Lu", &tval) != 1) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not fit \"unsigned long long\"", NULL);
    }
    return false;
  }

  da->insert_ulonglong ((CORBA::ULongLong) tval);
  return true;
}

bool
Combat_Packer::pack_Float (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  double tval;

  if (Tcl_GetDoubleFromObj (NULL, data, &tval) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"float\"", NULL);
    }
    return false;
  }

  da->insert_float ((CORBA::Float) tval);
  return true;
}

bool
Combat_Packer::pack_Double (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  double tval;

  if (Tcl_GetDoubleFromObj (NULL, data, &tval) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"double\"", NULL);
    }
    return false;
  }

  da->insert_double ((CORBA::Double) tval);
  return true;
}

bool
Combat_Packer::pack_LongDouble (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  double tval;

  if (Tcl_GetDoubleFromObj (NULL, data, &tval) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"double\"", NULL);
    }
    return false;
  }

  da->insert_longdouble ((CORBA::LongDouble) tval);
  return true;
}

bool
Combat_Packer::pack_Boolean (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  int tval;

  if (Tcl_GetBooleanFromObj (NULL, data, &tval) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"boolean\"", NULL);
    }
    return false;
  }

  da->insert_boolean ((tval) ? TRUE : FALSE);
  return true;
}

bool
Combat_Packer::pack_Char (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  char * tmp;
  int len;

  tmp = Tcl_GetStringFromObj (data, &len);

  if (len != 1) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp,
			"\" does not match \"char\"", NULL);
    }
    return false;
  }

  da->insert_char ((CORBA::Char) *tmp);
  return true;
}

bool
Combat_Packer::pack_Octet (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  int len;

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  char * tmp;
  tmp = Tcl_GetStringFromObj (data, &len);
#else
  unsigned char * tmp;
  tmp = Tcl_GetByteArrayFromObj (data, &len);
#endif

  if (len != 1) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp,
			"\" does not match \"octet\"", NULL);
    }
    return false;
  }

  da->insert_octet ((CORBA::Octet) *tmp);
  return true;
}

bool
Combat_Packer::pack_WChar (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  char * tmp;
  int len;

  tmp = Tcl_GetStringFromObj (data, &len);

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  if (len != sizeof (CORBA::WChar)) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp,
			"\" does not match \"wchar\"", NULL);
    }
    return false;
  }

  da->insert_wchar (*((CORBA::WChar *) tmp));
#else
  Tcl_UniChar ch;
  if (Tcl_UtfToUniChar (tmp, &ch) != len) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp,
			"\" does not match \"wchar\"", NULL);
    }
    return false;
  }

  da->insert_wchar ((CORBA::WChar) ch);
#endif

  return true;
}

bool
Combat_Packer::pack_TypeCode (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  CORBA::TypeCode_var ctc = Combat::GetTypeCodeFromObj (interp, data);

  if (CORBA::is_nil (ctc)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while packing TypeCode");
    }
    return false;
  }

  da->insert_typecode (ctc);
  return true;
}

bool
Combat_Packer::pack_Any (Tcl_Obj * data, DynamicAny::DynAny_ptr da)
{
  Tcl_Obj *ctc, *val;
  int len;

  if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &ctc) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &val) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: not an any value: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\"", NULL);
    }
    return false;
  }

  CORBA::TypeCode_var atc = Combat::GetTypeCodeFromObj (interp, ctc);

  if (CORBA::is_nil (atc)) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while packing TypeCode in an Any");
    }
    return false;
  }

  CORBA::Any_var cany = Combat::GetAnyFromObj (interp, ctx, val, atc);

  if (!cany) {
    if (interp) {
      Tcl_AddErrorInfo (interp, "\n  while packing contents of an Any");
    }
    return false;
  }

  da->insert_any (*cany);
  return true;
}

bool
Combat_Packer::pack_Objref (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			    DynamicAny::DynAny_ptr da)
{
  const char * str = Tcl_GetStringFromObj (data, NULL);
  CORBA::Object_var obj;

  assert (interp);
  assert (ctx);

  /*
   * We accept a "0" as nil reference or a handle
   */

  if (str[0] == '0' && str[1] == '\0') {
    obj = CORBA::Object::_nil ();
  }
  else {
    Tcl_CmdInfo info;

    if (!Tcl_GetCommandInfo (interp, (char *) str, &info)) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: no such object: ", str, NULL);
      return false;
    }

    Combat::Object * tobj = (Combat::Object *) info.objClientData;

    if (CORBA::is_nil (tobj->obj)) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: not an object: ", str, NULL);
      return false;
    }

    obj = CORBA::Object::_duplicate (tobj->obj);
  }

#if defined(COMBAT_USE_ORBACUS) && OB_INTEGER_VERSION >= 4010000
  if (strcmp (tc->id(), "IDL:omg.org/CORBA/Object:1.0") != 0)
#endif
  if (!CORBA::is_nil (obj) && !obj->_is_a (tc->id())) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: illegal type for object reference: ",
			"should be \"", tc->id(), "\"", NULL);
    }
    return false;
  }

  da->insert_reference (obj);
  return true;
}

bool
Combat_Packer::pack_Fixed (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			   DynamicAny::DynAny_ptr da)
{
  char * tmp = Tcl_GetStringFromObj (data, NULL);
  DynamicAny::DynFixed_var res = DynamicAny::DynFixed::_narrow (da);
  res->set_value (tmp);
  return true;
}

bool
Combat_Packer::pack_String (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			    DynamicAny::DynAny_ptr da)
{
  char * tmp;
  int len;

  tmp = Tcl_GetStringFromObj (data, &len);

  if (tc->length() && (CORBA::ULong) len > tc->length()) {
    if (interp) {
      char pl[64];
      sprintf (pl, "%lu", (unsigned long) tc->length());
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp, "\" exceeds boundary of ",
			"\"string<", pl, ">\"", NULL);
    }
    return false;
  }

  da->insert_string (tmp);
  return true;
}

bool
Combat_Packer::pack_WString (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			     DynamicAny::DynAny_ptr da)
{
  char * tmp;
  int len;

  tmp = Tcl_GetStringFromObj (data, &len);

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
  if ((tc->length() && (CORBA::ULong) (len-1) > tc->length()*sizeof(CORBA::WChar)) ||
      len % sizeof(CORBA::WChar) != 0) {
    if (interp) {
      char pl[64];
      sprintf (pl, "%lu", (unsigned long) tc->length());
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp, "\" exceeds boundary of ",
			"\"wstring<", pl, ">\"", NULL);
    }
    return false;
  }

  da->insert_wstring ((CORBA::WChar *) tmp);
#else
  Tcl_UniChar ch;
  CORBA::ULong cnt, total=0;
  CORBA::WString_var ws = CORBA::wstring_alloc (len);

  while (len > 0) {
    cnt = Tcl_UtfToUniChar (tmp, &ch);
    ws[total++] = (CORBA::WChar) ch;
    tmp += cnt;
    len -= cnt;
  }
  ws[total] = 0;

  if (tc->length() && total > tc->length()) {
    if (interp) {
      char pl[64];
      sprintf (pl, "%lu", (unsigned long) tc->length());
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"", tmp, "\" exceeds boundary of ",
			"\"wstring<", pl, ">\"", NULL);
    }
    return false;
  }

  da->insert_wstring (ws.in());
#endif

  return true;
}

bool
Combat_Packer::pack_Alias (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			   DynamicAny::DynAny_ptr da)
{
  const CORBA::TypeCode_var ntc = tc->content_type();
  bool res = Pack (data, ntc.in(), da);

  if (!res) {
    Tcl_AddErrorInfo (interp, "\n  while packing ");
    Tcl_AddErrorInfo (interp, (char *) tc->id ());
  }

  return res;
}

bool
Combat_Packer::pack_Struct (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			    DynamicAny::DynAny_ptr da, const char * name)
{
  CORBA::ULong i, len = tc->member_count();
  CORBA::Boolean r;
  int llen;

  if (!name) {
    name = "struct";
  }

  if (Tcl_ListObjLength (NULL, data, &llen) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" is not a structure, ",
			"was expecting \"", name, " ",
			tc->name(), "\"", NULL);
    }
    return false;
  }

  if (2*len != (CORBA::ULong) llen) {
    if (interp) {
      char sl[64], pl[64];
      sprintf (sl, "%lu", (unsigned long) 2*len);
      sprintf (pl, "%lu", (unsigned long) llen);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"", name, " ",
			tc->name(), "\": wrong # of elements",
			" (got ", pl, ", expected ", sl, ")",
			NULL);
    }
    return false;
  }

  /*
   * First, we must find out the correct order in which the elements
   * can be pushed into the any value.
   */

  CORBA::ULong * scramble = new CORBA::ULong [len];

  for (i=0; i<len; i++) {
    scramble[i] = (CORBA::ULong) -1;
  }

  for (i=0; i<len; i++) {
    const char * name, * mname;
    CORBA::ULong member;
    Tcl_Obj * mno;

    r = (Tcl_ListObjIndex (NULL, data, 2*i, &mno) == TCL_OK);
    assert (r);

    name  = Tcl_GetStringFromObj (mno, NULL);
    mname = tc->member_name (i);

    if (strcmp (name, mname) == 0) {
      member = i;
    }
    else {
      for (member=0; member<len; member++) {
	mname = tc->member_name (member);
	if (strcmp (name, mname) == 0) {
	  break;
	}
      }
      if (member >= len) {
	if (interp) {
	  Tcl_ResetResult (interp);
	  Tcl_AppendResult (interp, "error: \"", name,
			    "\" is not a member of \"", name, " ",
			    tc->name(), "\"", NULL);
	  Tcl_AddErrorInfo (interp, "\n  while packing \"");
	  Tcl_AddErrorInfo (interp, name);
	  Tcl_AddErrorInfo (interp, " ");
	  Tcl_AddErrorInfo (interp, tc->name());
	  Tcl_AddErrorInfo (interp, "\" from \"");
	  Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (data, NULL));
	  Tcl_AddErrorInfo (interp, "\"");
	}
	delete [] scramble;
	return false;
      }
    }

    if (scramble[member] != (CORBA::ULong) -1) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: member \"", name,
			  "\" appears twice", NULL);
	Tcl_AddErrorInfo (interp, "\n  while packing \"");
	Tcl_AddErrorInfo (interp, name);
	Tcl_AddErrorInfo (interp, " ");
	Tcl_AddErrorInfo (interp, tc->name());
	Tcl_AddErrorInfo (interp, "\" from \"");
	Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (data, NULL));
	Tcl_AddErrorInfo (interp, "\"");
      }
      delete [] scramble;
      return false;
    }

    scramble[member] = i;
  }

  DynamicAny::DynStruct_var res =
    DynamicAny::DynStruct::_narrow (da);

  for (i=0; i<len; i++) {
    Tcl_Obj * mvo;

    r = (Tcl_ListObjIndex (NULL, data, 2*(scramble[i])+1, &mvo) == TCL_OK);
    assert (r);

    CORBA::TypeCode_var ntc = tc->member_type (i);
    DynamicAny::DynAny_var cc = res->current_component ();

    if (!Pack (mvo, ntc.in(), cc.in())) {
      const char * mname = tc->member_name (i);
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while packing member \"");
	Tcl_AddErrorInfo (interp, mname);
	Tcl_AddErrorInfo (interp, "\" of \"");
	Tcl_AddErrorInfo (interp, name);
	Tcl_AddErrorInfo (interp, " ");
	Tcl_AddErrorInfo (interp, tc->name());
	Tcl_AddErrorInfo (interp, "\"");
      }
      delete [] scramble;
      return false;
    }

    res->next ();
  }

  delete [] scramble;
  return true;
}

bool
Combat_Packer::pack_Except (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			    DynamicAny::DynAny_ptr da)
{
  Tcl_Obj *id, *val;
  const char *rid;
  int llen;

  if (Tcl_ListObjLength (NULL, data, &llen) != TCL_OK ||
      (llen != 1 && llen != 2) ||
      Tcl_ListObjIndex (NULL, data, 0, &id) != TCL_OK ||
      (rid = Tcl_GetStringFromObj (id, NULL)) == NULL) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" is not an exception, ",
			"was expecting \"exception ",
			tc->id(), "\"", NULL);
    }
    return false;
  }

  if (strcmp (rid, tc->id()) != 0) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: was expecting \"exception ",
			tc->id(), "\" but got \"", rid, "\"", NULL);
    }
    return false;
  }

  if (llen == 1) {
    if (tc->member_count() != 0) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\" does not match \"exception ",
			  tc->name(), "\": wrong # of elements",
			  " (expected 0)",
			  NULL);
      }
      return false;
    }

    return true;
  }

  Tcl_ListObjIndex (NULL, data, 1, &val);
  return pack_Struct (val, tc, da, "exception");
}

bool
Combat_Packer::pack_Sequence (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			      DynamicAny::DynAny_ptr da)
{
  CORBA::Boolean r;
  Tcl_Obj * ts;
  int i, llen;

  DynamicAny::DynSequence_var res =
    DynamicAny::DynSequence::_narrow (da);
  CORBA::TypeCode_var ctc = tc->content_type();

  /*
   * Special handling for octet and char sequences
   */

  CORBA::TypeCode_var uatc = CORBA::TypeCode::_duplicate (ctc);
  while (uatc->kind() == CORBA::tk_alias)
    uatc = uatc->content_type ();

  if (uatc->kind() == CORBA::tk_octet || uatc->kind() == CORBA::tk_char) {
    CORBA::Octet * buf;
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
    buf = (CORBA::Octet *) Tcl_GetStringFromObj (data, &llen);
#else
    buf = (CORBA::Octet *) Tcl_GetByteArrayFromObj (data, &llen);
#endif
    if (tc->length() && (CORBA::ULong) llen > tc->length()) {
      if (interp) {
	Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\" exceeds bound of \"",
			  Tcl_GetStringFromObj (name, NULL),
			  "\"", NULL);
	Tcl_DecrRefCount (name);
      }
      return false;
    }

    /*
     * There's no simple CORBA-compliant way of getting a lot of octets
     * at once into the DynAny if the sequence is bounded, as sequence
     * typecodes with a different length aren't equivalent, and from_any
     * would fail with BAD_TYPECODE. Bummer.
     * And ORBacus does not have an Any insertion operator for OctetSeq.
     */

#if defined(COMBAT_USE_MICO)
    if (tc->length()) {
#endif
      res->set_length (llen);
      switch (uatc->kind()) {
      case CORBA::tk_octet:
	{
	  for (i=0; i<llen; i++) {
	    res->insert_octet (buf[i]);
	    res->next ();
	  }
	}
        break;
      case CORBA::tk_char:
	{
	  for (i=0; i<llen; i++) {
	    res->insert_char (buf[i]);
	    res->next ();
	  }
	}
        break;
      default:
	assert (0);
      }
      return true;
#if defined(COMBAT_USE_MICO)
    }
    CORBA::Any any;
    switch (uatc->kind()) {
    case CORBA::tk_octet:
      {
	CORBA::OctetSeq os (llen, llen, buf);
	any <<= os;
      }
      break;
    case CORBA::tk_char:
      {
	CORBA::CharSeq cs (llen, llen, (CORBA::Char *) buf);
	any <<= cs;
      }
      break;
    default:
      assert (0);
    }
    res->from_any (any);
    return true;
#endif
  }

  if (Tcl_ListObjLength (NULL, data, &llen) != TCL_OK) {
    if (interp) {
      Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" is not a sequence, ",
			"was expecting \"",
			Tcl_GetStringFromObj (name, NULL),
			"\"", NULL);
      Tcl_DecrRefCount (name);
    }
    return false;
  }

  if (tc->length() && (CORBA::ULong) llen > tc->length()) {
    if (interp) {
      Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" exceeds bound of \"",
			Tcl_GetStringFromObj (name, NULL),
			"\"", NULL);
      Tcl_DecrRefCount (name);
    }
    return false;
  }

  res->set_length (llen);
  for (i=0; i<llen; i++) {
    r = (Tcl_ListObjIndex (NULL, data, i, &ts) == TCL_OK);
    assert (r);

    DynamicAny::DynAny_var cc = res->current_component ();

    if (!Pack (ts, ctc.in(), cc.in())) {
      if (interp) {
	Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
	char ind[64];
	sprintf (ind, "%lu", (unsigned long) i);
	Tcl_AddErrorInfo (interp, "\n  while packing item # ");
	Tcl_AddErrorInfo (interp, ind);
	Tcl_AddErrorInfo (interp, " of \"");
	Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (name, NULL));
	Tcl_AddErrorInfo (interp, "\"");
	Tcl_DecrRefCount (name);
      }
      return false;
    }
    
    res->next ();
  }

  return true;
}

bool
Combat_Packer::pack_Array (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			   DynamicAny::DynAny_ptr da)
{
  CORBA::Boolean r;
  Tcl_Obj * ts;
  int i, llen;

  DynamicAny::DynArray_var res =
    DynamicAny::DynArray::_narrow (da);
  CORBA::TypeCode_var ctc = tc->content_type();

  /*
   * Special handling for octet and char arrays
   */

  CORBA::TypeCode_var uatc = CORBA::TypeCode::_duplicate (ctc);
  while (uatc->kind() == CORBA::tk_alias)
    uatc = uatc->content_type ();

  if (uatc->kind() == CORBA::tk_octet || uatc->kind() == CORBA::tk_char) {
    CORBA::Octet * buf;
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
    buf = (CORBA::Octet *) Tcl_GetStringFromObj (data, &llen);
#else
    buf = (CORBA::Octet *) Tcl_GetByteArrayFromObj (data, &llen);
#endif
    if (tc->length() != (CORBA::ULong) llen) {
      if (interp) {
	Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: \"",
			  Tcl_GetStringFromObj (data, NULL),
			  "\" does not match item count of \"",
			  Tcl_GetStringFromObj (name, NULL),
			  "\"", NULL);
	Tcl_DecrRefCount (name);
      }
      return false;
    }
    switch (uatc->kind()) {
    case CORBA::tk_octet:
      {
	for (i=0; i<llen; i++) {
	  res->insert_octet (buf[i]);
	  res->next ();
	}
      }
      break;
    case CORBA::tk_char:
      {
	for (i=0; i<llen; i++) {
	  res->insert_char (buf[i]);
	  res->next ();
	}
      }
      break;
    default:
      assert (0);
    }
    return true;
  }

  if (Tcl_ListObjLength (NULL, data, &llen) != TCL_OK) {
    if (interp) {
      Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" is not an array, ",
			"was expecting \"",
			Tcl_GetStringFromObj (name, NULL),
			"\"", NULL);
      Tcl_DecrRefCount (name);
    }
    return false;
  }

  if ((CORBA::ULong) llen != tc->length()) {
    if (interp) {
      Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match item count of \"",
			Tcl_GetStringFromObj (name, NULL),
			"\"", NULL);
      Tcl_DecrRefCount (name);
    }
    return false;
  }

  for (i=0; i<llen; i++) {
    r = (Tcl_ListObjIndex (NULL, data, i, &ts) == TCL_OK);
    assert (r);

    DynamicAny::DynAny_var cc = res->current_component ();

    if (!Pack (ts, ctc.in(), cc.in())) {
      if (interp) {
	Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
	char ind[64];
	sprintf (ind, "%lu", (unsigned long) i);
	Tcl_AddErrorInfo (interp, "\n  while packing item # ");
	Tcl_AddErrorInfo (interp, ind);
	Tcl_AddErrorInfo (interp, " of \"");
	Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (name, NULL));
	Tcl_AddErrorInfo (interp, "\"");
	Tcl_DecrRefCount (name);
      }
      return false;
    }
    
    res->next ();
  }

  return true;
}

bool
Combat_Packer::pack_Enum (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			  DynamicAny::DynAny_ptr da)
{
  char * tmp = Tcl_GetStringFromObj (data, NULL);
  DynamicAny::DynEnum_var res =
    DynamicAny::DynEnum::_narrow (da);

  for (CORBA::ULong i=0; i < tc->member_count(); i++) {
    if (strcmp (tmp, tc->member_name (i)) == 0) {
      res->set_as_ulong (i);
      return true;
    }
  }

  if (interp) {
    Tcl_ResetResult (interp);
    Tcl_AppendResult (interp, "error: \"",
                      Tcl_GetStringFromObj (data, NULL),
                      "\" is not member of enum \"",
                      tc->name (), "\"", NULL);
  }

  return false;
}

bool
Combat_Packer::pack_Union (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			   DynamicAny::DynAny_ptr da)
{
  CORBA::TypeCode_var dt = tc->discriminator_type ();
  Tcl_Obj *disc, *memb;
  int llen;

  if (Tcl_ListObjLength (NULL, data, &llen) != TCL_OK || llen != 2 ||
      Tcl_ListObjIndex (NULL, data, 0, &disc) != TCL_OK ||
      Tcl_ListObjIndex (NULL, data, 1, &memb) != TCL_OK) {
    if (interp) {
      Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" is not a union, ",
			"was expecting \"",
			Tcl_GetStringFromObj (name, NULL),
			"\"", NULL);
      Tcl_DecrRefCount (name);
    }
    return false;
  }

  DynamicAny::DynUnion_var res =
    DynamicAny::DynUnion::_narrow (da);
  
  if (strcmp (Tcl_GetStringFromObj (disc, NULL), "(default)") == 0) {
    res->set_to_default_member ();
  }
  else {
    DynamicAny::DynAny_var dadis = res->get_discriminator ();

    if (!Pack (disc, dt.in(), dadis.in())) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while packing discriminator ");
	Tcl_AddErrorInfo (interp, "of union \"");
	Tcl_AddErrorInfo (interp, (char *) tc->id ());
	Tcl_AddErrorInfo (interp, "\"");
      }
      return false;
    }
  }

  if (res->has_no_active_member()) {
    int ilen;
    if (Tcl_ListObjLength (NULL, memb, &ilen) != TCL_OK || ilen != 0) {
      if (interp) {
	Tcl_ResetResult (interp);
	Tcl_AppendResult (interp, "error: expecting empty union, got \"",
			  Tcl_GetStringFromObj (memb, NULL), "\"", NULL);
	Tcl_AddErrorInfo (interp, "\n  while packing member of union \"");
	Tcl_AddErrorInfo (interp, (char *) tc->id ());
	Tcl_AddErrorInfo (interp, "\" for discriminator \"");
	Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (disc, NULL));
	Tcl_AddErrorInfo (interp, "\"");
      }
      return false;
    }
  }
  else {
    DynamicAny::DynAny_var mc = res->member ();
    CORBA::TypeCode_var mtype = mc->type ();

    if (!Pack (memb, mtype.in(), mc.in())) {
      if (interp) {
	Tcl_AddErrorInfo (interp, "\n  while packing member of union \"");
	Tcl_AddErrorInfo (interp, (char *) tc->id ());
	Tcl_AddErrorInfo (interp, "\" for discriminator \"");
	Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (disc, NULL));
	Tcl_AddErrorInfo (interp, "\"");
      }
      return false;
    }
  }

  return true;
}

#if (!defined(COMBAT_USE_ORBACUS) || OB_INTEGER_VERSION >= 4010000) && \
	(!defined(COMBAT_USE_MICO) || MICO_BIN_VERSION >= 0x020306)

bool
Combat_Packer::pack_Value (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			   DynamicAny::DynAny_ptr da)
{
  DynamicAny::DynValue_var dv = DynamicAny::DynValue::_narrow (da);

  /*
   * See if this valuetype is supposed to be null
   */

  const char * sdata = Tcl_GetStringFromObj (data, NULL);

  if (strcmp (sdata, "0") == 0) {
    dv->set_to_null ();
    return true;
  }

  dv->set_to_value ();

  /*
   * Elements must be packed from the innermost element outwards
   */

  std::vector<CORBA::TypeCode_var> bases;
  CORBA::TypeCode_var iter = CORBA::TypeCode::_duplicate (tc);
  CORBA::ULong len = 0;

  while (!CORBA::is_nil (iter)) {
    bases.push_back (iter);
    len += iter->member_count ();
    iter = iter->concrete_base_type ();
  }

  CORBA::Boolean r;
  int llen;

  if (Tcl_ListObjLength (NULL, data, &llen) != TCL_OK) {
    if (interp) {
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" is not a valuetype, ",
			"was expecting \"valuetype ",
			tc->name(), "\"", NULL);
    }
    return false;
  }

  if (2*len > (CORBA::ULong) llen) {
    if (interp) {
      char sl[64], pl[64];
      sprintf (sl, "%lu", (unsigned long) 2*len);
      sprintf (pl, "%lu", (unsigned long) llen);
      Tcl_ResetResult (interp);
      Tcl_AppendResult (interp, "error: \"",
			Tcl_GetStringFromObj (data, NULL),
			"\" does not match \"valuetype ",
			tc->name(), "\": wrong # of elements",
			" (got ", pl, ", expected at least ", sl, ")",
			NULL);
    }
    return false;
  }

  /*
   * First, we must find out the correct order in which the elements
   * can be pushed into the any value.
   */

  CORBA::ULong * scramble = new CORBA::ULong [len];
  CORBA::ULong i;

  for (i=0; i<len; i++) {
    scramble[i] = (CORBA::ULong) -1;
  }

  std::vector<CORBA::TypeCode_var>::iterator it = bases.end ();

  if (it != bases.begin()) {
    i = 0;

    do {
      it--;

      for (CORBA::ULong j=0; j<(*it)->member_count(); j++, i++) {
	const char * name, * mname;
	CORBA::ULong member;
	Tcl_Obj * mno;

	r = (Tcl_ListObjIndex (NULL, data, 2*i, &mno) == TCL_OK);
	assert (r);
	name = Tcl_GetStringFromObj (mno, NULL);
	mname = (*it)->member_name (j);

	if (strcmp (name, mname) == 0) {
	  member = 2*i;
	}
	else {
	  for (member=0; member<llen; member+=2) {
	    r = (Tcl_ListObjIndex (NULL, data, member, &mno) == TCL_OK);
	    assert (r);

	    name = Tcl_GetStringFromObj (mno, NULL);

	    if (strcmp (name, mname) == 0) {
	      break;
	    }
	  }
	}

	if (member >= llen) {
	  if (interp) {
	    Tcl_ResetResult (interp);
	    Tcl_AppendResult (interp, "error: \"", name,
			      "\" is not a member of \"", name, " ",
			      tc->name(), "\"", NULL);
	  }
	  delete [] scramble;
	  return false;
	}

	if (scramble[i] != (CORBA::ULong) -1) {
	  if (interp) {
	    Tcl_ResetResult (interp);
	    Tcl_AppendResult (interp, "error: member \"", name,
			      "\" appears twice while setting \"",
			      name, " ", tc->name(), "\" from \"",
			      Tcl_GetStringFromObj (data, NULL), "\"", NULL);
	  }
	  delete [] scramble;
	  return false;
	}

	scramble[i] = member;
      }
    }
    while (it != bases.begin());
  }

  i = 0;

  while (bases.size() > 0) {
    iter = bases.back ();
    bases.pop_back ();

    CORBA::ULong len = iter->member_count ();

    for (CORBA::ULong j=0; j<len; j++, i++) {
      Tcl_Obj * mvo;

      r = (Tcl_ListObjIndex (NULL, data, scramble[i]+1, &mvo) == TCL_OK);
      assert (r);

      CORBA::TypeCode_var ntc = iter->member_type (j);
      DynamicAny::DynAny_var cc = dv->current_component ();

      if (!Pack (mvo, ntc.in(), cc.in())) {
	const char * mname = iter->member_name (j);
	if (interp) {
	  Tcl_AddErrorInfo (interp, "\n  while packing member \"");
	  Tcl_AddErrorInfo (interp, mname);
	  Tcl_AddErrorInfo (interp, "\" of \"valuetype ");
	  Tcl_AddErrorInfo (interp, iter->name());
	  Tcl_AddErrorInfo (interp, "\"");
	}
	delete [] scramble;
	return false;
      }

      dv->next ();
    }
  }

  delete [] scramble;
  return true;
}

bool
Combat_Packer::pack_ValueBox (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
			      DynamicAny::DynAny_ptr da)
{
  DynamicAny::DynValueBox_var dv = DynamicAny::DynValueBox::_narrow (da);

  /*
   * See if this valuetype is supposed to be null
   */

  const char * sdata = Tcl_GetStringFromObj (data, NULL);

  if (strcmp (sdata, "0") == 0) {
    dv->set_to_null ();
    return true;
  }

  dv->set_to_value ();

  DynamicAny::DynAny_var ca = dv->get_boxed_value_as_dyn_any ();
  const CORBA::TypeCode_var ntc = tc->content_type ();
  bool res = Pack (data, ntc.in(), da);

  if (!res) {
    Tcl_AddErrorInfo (interp, "\n  while packing ");
    Tcl_AddErrorInfo (interp, (char *) ntc->id ());
  }

  return res;
}

#else

bool
Combat_Packer::pack_Value (Tcl_Obj *, const CORBA::TypeCode_ptr,
			   DynamicAny::DynAny_ptr)
{
  assert (0);
  return 0;
}

bool
Combat_Packer::pack_ValueBox (Tcl_Obj *, const CORBA::TypeCode_ptr,
			      DynamicAny::DynAny_ptr)
{
  assert (0);
  return 0;
}

#endif

/*
 * Packer Main
 */

bool
Combat_Packer::Pack (Tcl_Obj * data, const CORBA::TypeCode_ptr tc,
		     DynamicAny::DynAny_ptr value)
{
  bool res;

#ifdef HAVE_EXCEPTIONS
  try {
#endif

  /*
   * Maybe data is of our Any type?
   */

  if (data->typePtr == &Combat::AnyType) {
    TclAnyData * objInf = (TclAnyData *) data->internalRep.otherValuePtr;
    if (!CORBA::is_nil (objInf->any)) {
      CORBA::TypeCode_var objtc = objInf->any->type ();
      if (tc->equal (objtc)) {
	value->assign (objInf->any);
	return true;
      }
    }
  }

  /*
   * Else go the hard way
   */

  switch (tc->kind()) {
    
    /*
     * Primitive types
     */

  case CORBA::tk_null:
  case CORBA::tk_void:
    {
      int len;
      if (Tcl_ListObjLength (NULL, data, &len) != TCL_OK || len != 0) {
	if (interp) {
	  Tcl_ResetResult (interp);
	  Tcl_AppendResult (interp, "error: expecting void, but got \"",
			    Tcl_GetStringFromObj (data, NULL), "\"",
			    NULL);
	}
	return false;
      }
      res = true;
    }
    break;

  case CORBA::tk_short:
    res = pack_Short (data, value);
    break;

  case CORBA::tk_long:
    res = pack_Long (data, value);
    break;

  case CORBA::tk_ushort:
    res = pack_UShort (data, value);
    break;

  case CORBA::tk_ulong:
    res = pack_ULong (data, value);
    break;

  case CORBA::tk_float:
    res = pack_Float (data, value);
    break;

  case CORBA::tk_double:
    res = pack_Double (data, value);
    break;

  case CORBA::tk_boolean:
    res = pack_Boolean (data, value);
    break;

  case CORBA::tk_char:
    res = pack_Char (data, value);
    break;

  case CORBA::tk_octet:
    res = pack_Octet (data, value);
    break;

  case CORBA::tk_wchar:
    res = pack_WChar (data, value);
    break;

  case CORBA::tk_longlong:
    res = pack_LongLong (data, value);
    break;

  case CORBA::tk_ulonglong:
    res = pack_ULongLong (data, value);
    break;

  case CORBA::tk_longdouble:
    res = pack_LongDouble (data, value);
    break;

  case CORBA::tk_TypeCode:
    res = pack_TypeCode (data, value);
    break;

  case CORBA::tk_Principal:
    assert (0);
    break;

  case CORBA::tk_any:
    res = pack_Any (data, value);
    break;
    
    /*
     * Complex types
     */

  case CORBA::tk_objref:
    res = pack_Objref (data, tc, value);
    break;

  case CORBA::tk_string:
    res = pack_String (data, tc, value);
    break;

  case CORBA::tk_wstring:
    res = pack_WString (data, tc, value);
    break;

  case CORBA::tk_fixed:
    res = pack_Fixed (data, tc, value);
    break;

  case CORBA::tk_alias:
    res = pack_Alias (data, tc, value);
    break;

  case CORBA::tk_struct:
    res = pack_Struct (data, tc, value);
    break;

  case CORBA::tk_sequence:
    res = pack_Sequence (data, tc, value);
    break;

  case CORBA::tk_array:
    res = pack_Array (data, tc, value);
    break;

  case CORBA::tk_enum:
    res = pack_Enum (data, tc, value);
    break;

  case CORBA::tk_union:
    res = pack_Union (data, tc, value);
    break;

  case CORBA::tk_except:
    res = pack_Except (data, tc, value);
    break;

  case CORBA::tk_value:
    res = pack_Value (data, tc, value);
    break;

  case CORBA::tk_value_box:
    res = pack_ValueBox (data, tc, value);
    break;

  default:
    assert (0);
  }

#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::Exception &ex) {
    if (interp) {
      Tcl_SetObjResult (interp, Combat::DecodeException (interp, ctx, &ex));
    }
    return false;
  }
#endif

  if (!res) {
    if (interp) {
      Tcl_Obj * name = Combat::NewTypeCodeObj (tc);
      Tcl_AddErrorInfo (interp, "\n  while packing \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (name, NULL));
      Tcl_AddErrorInfo (interp, "\" from \"");
      Tcl_AddErrorInfo (interp, Tcl_GetStringFromObj (data, NULL));
      Tcl_AddErrorInfo (interp, "\"");
      Tcl_DecrRefCount (name);
    }
  }

  return res;
}

/*
 * Public Packer main
 */

DynamicAny::DynAny_ptr
Combat_Packer::Pack (Tcl_Obj * data, const CORBA::TypeCode_ptr tc)
{
  DynamicAny::DynAny_ptr res =
    Combat::GlobalData->daf->create_dyn_any_from_type_code (tc);
  if (!Pack (data, tc, res)) {
    res->destroy ();
    CORBA::release (res);
    return DynamicAny::DynAny::_nil ();
  }
  return res;
}
