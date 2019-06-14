#include "server.h"
#include <fstream>
#include <string.h>

using namespace std;

class diamonda_impl : virtual public POA_diamonda
{
public:
  char * opa ()
  {
    return CORBA::string_dup ("opa");
  };
};

class diamondb_impl :
  virtual public diamonda_impl,
  virtual public POA_diamondb
{
public:
  char * opb ()
  {
    return CORBA::string_dup ("opb");
  };
};

class diamondc_impl :
  virtual public diamonda_impl,
  virtual public POA_diamondc
{
public:
  char * opc ()
  {
    return CORBA::string_dup ("opc");
  };
};

class diamondd_impl :
  virtual public diamondb_impl,
  virtual public diamondc_impl,
  virtual public POA_diamondd
{
public:
  char * opd ()
  {
    return CORBA::string_dup ("opd");
  };
};

class operations_impl : virtual public POA_operations
{
private:
  CORBA::Short _s;

public:
  CORBA::Short s ()
  {
    return _s;
  };

  void s (CORBA::Short __s)
  {
    _s = __s;
  };

  char * ra ()
  {
    return CORBA::string_dup ("Hello World");
  };

  CORBA::ULong square (CORBA::Short x)
  {
    return (CORBA::ULong) x * x;
  };

  CORBA::Long copy (const char * sin, CORBA::String_out sout)
  {
    sout = CORBA::string_dup (sin);
    return strlen (sin);
  };

  CORBA::UShort length (const Q & queue, E & oe)
  {
    oe = ((queue.length() % 2) == 0) ? EVEN : ODD;
    return queue.length();
  };

  Q * squares (CORBA::UShort howmany)
  {
    Q * q = new Q;
    q->length (howmany);
    for (CORBA::UShort u=0; u<howmany; u++) {
      (*q)[u].member = u*u;
    }
    return q;
  };

  void reverse (char * & str)
  {
    str = CORBA::string_dup (str);
    int l = strlen (str);

    for (int i=0; i<l/2; i++) {
      char tmp = str[i];
      str[i] = str[l-i-1];
      str[l-i-1] = tmp;
    }
  };

  void nop ()
  {
  };

  operations * dup ()
  {
    return _this ();
  };

  void dup2 (CORBA::Object_ptr o1, CORBA::Object_out o2)
  {
    o2 = CORBA::Object::_duplicate (o1);
  }

  CORBA::Boolean isme (CORBA::Object_ptr obj)
  {
    CORBA::Object_var me = _this ();
    return me->_is_equivalent (obj);
  };

  diamond * getdiamond ()
  {
    diamond * diam = new diamond;

    diamonda_impl * da = new diamonda_impl;
    diamondb_impl * db = new diamondb_impl;
    diamondc_impl * dc = new diamondc_impl;
    diamondd_impl * dd = new diamondd_impl;

    diam->a = da->_this ();
    diam->b = db->_this ();
    diam->c = dc->_this ();
    diam->d = dd->_this ();

    diam->abcd.length (4);
    diam->abcd[0] = da->_this ();
    diam->abcd[1] = db->_this ();
    diam->abcd[2] = dc->_this ();
    diam->abcd[3] = dd->_this ();

    return diam;
  }

  void DontCallMe ()
  {
    throw Oops ("I said, don't call me!");
  }
};

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  operations_impl * pt = new operations_impl;
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
