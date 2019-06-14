#include "server.h"
#include <fstream>

using namespace std;

/*
 * EmptyValue implementation
 */

class EmptyValue_impl :
  virtual public OBV_EmptyValue,
  virtual public CORBA::DefaultValueRefCountBase
{
public:
  EmptyValue_impl ()
    {
    }
};

class EmptyValue_Factory : virtual public CORBA::ValueFactoryBase
{
public:
  CORBA::ValueBase * create_for_unmarshal ()
    {
      return new EmptyValue_impl;
    }
};

/*
 * Date implementation
 */

class Date_impl :
  virtual public OBV_Date,
  virtual public CORBA::DefaultValueRefCountBase
{
public:
  Date_impl ()
    {
    }
};

class Date_Factory : virtual public CORBA::ValueFactoryBase
{
public:
  CORBA::ValueBase * create_for_unmarshal ()
    {
      return new Date_impl;
    }
};

/*
 * BaseType implementation
 */

class BaseType_impl :
  virtual public OBV_BaseType,
  virtual public CORBA::DefaultValueRefCountBase
{
public:
  BaseType_impl ()
    {
      name ((const char *) "");
    }
};

class BaseType_Factory : virtual public CORBA::ValueFactoryBase
{
public:
  CORBA::ValueBase * create_for_unmarshal ()
    {
      return new BaseType_impl;
    }
};

/*
 * DerivedType implementation
 */

class DerivedType_impl :
  virtual public OBV_DerivedType,
  virtual public BaseType_impl,
  virtual public CORBA::DefaultValueRefCountBase
{
public:
  DerivedType_impl ()
    {
    }
};

class DerivedType_Factory : virtual public CORBA::ValueFactoryBase
{
public:
  CORBA::ValueBase * create_for_unmarshal ()
    {
      return new DerivedType_impl;
    }
};

/*
 * TreeNode implementation
 */

class TreeNode_impl :
  virtual public OBV_TreeNode,
  virtual public CORBA::DefaultValueRefCountBase
{
public:
  TreeNode_impl ()
    {
      nv (0);
      left (0);
      right (0);
    }
};

class TreeNode_Factory : virtual public CORBA::ValueFactoryBase
{
public:
  CORBA::ValueBase * create_for_unmarshal ()
    {
      return new TreeNode_impl;
    }
};

/*
 * Interface Implementation
 */

class ValueTest_impl : virtual public POA_ValueTest
{
private:
  EmptyValue * _ev;
  Date * _d;
  BaseType * _bt;
  DerivedType * _dt;
  TreeNode * _tn;
  CORBA::ValueBase * _vb;

public:
  EmptyValue * ev () { CORBA::add_ref (_ev); return _ev; }
  void ev (EmptyValue * v) { CORBA::add_ref (v); _ev = v; }

  Date * d () { CORBA::add_ref (_d); return _d; }
  void d (Date * v) { CORBA::add_ref (v); _d = v; }

  BaseType * bt () { CORBA::add_ref (_bt); return _bt; }
  void bt (BaseType * v) { CORBA::add_ref (v); _bt = v; }

  DerivedType * dt () { CORBA::add_ref (_dt); return _dt; }
  void dt (DerivedType * v) { CORBA::add_ref (v); _dt = v; }

  TreeNode * tn () { CORBA::add_ref (_tn); return _tn; }
  void tn (TreeNode * v) { CORBA::add_ref (v); _tn = v; }

  CORBA::ValueBase * vb () { CORBA::add_ref (_vb); return _vb; }
  void vb (CORBA::ValueBase * v) { CORBA::add_ref (v); _vb = v; }
};

/*
 * main()
 */

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  CORBA::ValueFactory vf = new EmptyValue_Factory;
  orb->register_value_factory ("IDL:EmptyValue:1.0", vf);

  vf = new Date_Factory;
  orb->register_value_factory ("IDL:Date:1.0", vf);

  vf = new BaseType_Factory;
  orb->register_value_factory ("IDL:BaseType:1.0", vf);

  vf = new DerivedType_Factory;
  orb->register_value_factory ("IDL:DerivedType:1.0", vf);

  vf = new TreeNode_Factory;
  orb->register_value_factory ("IDL:TreeNode:1.0", vf);

  ValueTest_impl * pt = new ValueTest_impl;
  PortableServer::ObjectId_var oid = poa->activate_object (pt);

  ofstream of ("server.ior");
  CORBA::Object_var ref = poa->id_to_reference (oid.in());
  CORBA::String_var str = orb->object_to_string (ref.in());
  of << str.in() << endl;
  of.close ();

  mgr->activate ();
  orb->run ();

  return 0;
}
