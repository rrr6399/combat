/*
 * A simple "Hello World" example that uses the POA
 */

#include "server.h"
#include <fstream.h>
#include <stdio.h>

/*
 * Hello World implementation inherits the POA skeleton class
 */

class HelloWorld_impl : virtual public POA_HelloWorld
{
public:
  void hello ();
};

void
HelloWorld_impl::hello ()
{
  printf ("Hello World\n");
}

int
main (int argc, char *argv[])
{
  /*
   * Initialize the ORB
   */

  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);

  /*
   * Obtain a reference to the RootPOA and its Manager
   */

  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  /*
   * Create a Hello World object
   */

  HelloWorld_impl * hello = new HelloWorld_impl;

  /*
   * Activate the Servant
   */

  PortableServer::ObjectId_var oid = poa->activate_object (hello);

  /*
   * Write IOR to file
   */

  ofstream of ("server.ior");
  CORBA::Object_var ref = poa->id_to_reference (oid.in());
  CORBA::String_var str = orb->object_to_string (ref.in());
  of << str.in() << endl;
  of.close ();

  /*
   * Activate the POA and start serving requests
   */

  printf ("Running.\n");

  mgr->activate ();
  orb->run();

  /*
   * Shutdown (never reached)
   */

  poa->destroy (1, 1);
  delete hello;

  return 0;
}
