#include "server.h"
#include <fstream>

using namespace std;

class Simple_impl : virtual public POA_Simple
{
public:
  void hello ()
  {
  }
};

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  Simple_impl * simple = new Simple_impl;
  PortableServer::ObjectId_var oid = poa->activate_object (simple);

  ofstream of ("server.ior");
  CORBA::Object_var ref = poa->id_to_reference (oid.in());
  CORBA::String_var str = orb->object_to_string (ref.in());
  of << str.in() << endl;
  of.close ();

  mgr->activate ();
  orb->run ();

  return 0;
}
