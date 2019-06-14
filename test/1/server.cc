#include "server.h"
#include <fstream>

using namespace std;

class ptypes_impl : virtual public POA_ptypes
{
private:
  CORBA::Short _s;
  CORBA::UShort _us;
  CORBA::Long _l;
  CORBA::ULong _ul;
  CORBA::Char _c;
  CORBA::Octet _o;
  CORBA::Boolean _b;
  CORBA::Float _f;
  CORBA::Double _d;
  CORBA::String_var _q;
  CORBA::WChar _wc;
  CORBA::WString_var _ws;

public:
  CORBA::Short s () { return _s; };
  void s (CORBA::Short __s) { _s = __s; };

  CORBA::UShort us () { return _us; };
  void us (CORBA::UShort __us) { _us = __us; };

  CORBA::Long l () { return _l; };
  void l (CORBA::Long __l) { _l = __l; };

  CORBA::ULong ul () { return _ul; };
  void ul (CORBA::ULong __ul) { _ul = __ul; };

  CORBA::Char c () { return _c; };
  void c (CORBA::Char __c) { _c = __c; };

  CORBA::Octet o () { return _o; };
  void o (CORBA::Octet __o) { _o = __o; };

  CORBA::Boolean b () { return _b; };
  void b (CORBA::Boolean __b) { _b = __b; };

  CORBA::Float f () { return _f; };
  void f (CORBA::Float __f) { _f = __f; };

  CORBA::Double d () { return _d; };
  void d (CORBA::Double __d) { _d = __d; };

  char * q () { return CORBA::string_dup (_q.in()); }
  void q (const char * __q) { _q = __q; };

  CORBA::WChar wc () { return _wc; };
  void wc (CORBA::WChar __wc) { _wc = __wc; };

  CORBA::WChar * ws () { return CORBA::wstring_dup (_ws.in()); };
  void ws (const CORBA::WChar * __ws) { _ws = __ws; };

  CORBA::ULong ro () { return 4242; };
};

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  ptypes_impl * pt = new ptypes_impl;
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
