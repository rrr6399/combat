#include "server.h"
#include <fstream>

using namespace std;

class AnyTest_impl : virtual public POA_AnyTest
{
private:
  CORBA::Any _value;

public:
  CORBA::Any * value ();
  void value (const CORBA::Any &);
};

CORBA::Any *
AnyTest_impl::value ()
{
  return new CORBA::Any (_value);
}

void
AnyTest_impl::value (const CORBA::Any & _v)
{
  _value = _v;
}

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  AnyTest_impl * at = new AnyTest_impl;
  PortableServer::ObjectId_var oid = poa->activate_object (at);

  ofstream of ("server.ior");
  CORBA::Object_var ref = poa->id_to_reference (oid.in());
  CORBA::String_var str = orb->object_to_string (ref.in());
  of << str.in() << endl;
  of.close ();

  mgr->activate ();
  orb->run ();

  return 0;
}
