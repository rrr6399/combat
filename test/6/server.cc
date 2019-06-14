#include "server.h"
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

using namespace std;

class Async_impl : virtual public POA_Async
{
public:
  CORBA::UShort sleep (CORBA::UShort seconds)
  {
    ::sleep (seconds);
    return seconds;
  };

  CORBA::ULong strcpy (CORBA::String_out dest, const char * source)
  {
    dest = CORBA::string_dup (source);
    return strlen (source);
  };

  void nop (void)
  {
  };
};

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  Async_impl * as = new Async_impl;
  PortableServer::ObjectId_var oid = poa->activate_object (as);

  char name[256];
  sprintf (name, "server_%s.ior", argv[1]);
  ofstream of (name);
  CORBA::Object_var ref = poa->id_to_reference (oid.in());
  CORBA::String_var str = orb->object_to_string (ref.in());
  of << str.in() << endl;
  of.close ();

  mgr->activate ();
  orb->run ();

  return 0;
}
