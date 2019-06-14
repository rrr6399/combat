#include "server.h"
#include <fstream>

using namespace std;

class unions_impl : virtual public POA_unions
{
private:
  Union1 _u1;
  Union2 _u2;
  Union3 _u3;
  Union4 _u4;
  Union5 _u5;
  Union6 _u6;
  Union7 _u7;

public:
  Union1 * u1 () { return new Union1(_u1); };
  void u1 (const Union1 & __u1) { _u1 = __u1; };

  Union2 * u2 () { return new Union2(_u2); };
  void u2 (const Union2 & __u2) { _u2 = __u2; };

  Union3 u3 () { return _u3; };
  void u3 (const Union3 & __u3) { _u3 = __u3; };

  Union4 u4 () { return _u4; };
  void u4 (const Union4 & __u4) { _u4 = __u4; };

  Union5 * u5 () { return new Union5(_u5); };
  void u5 (const Union5 & __u5) { _u5 = __u5; };

  Union6 * u6 () { return new Union6(_u6); };
  void u6 (const Union6 & __u6) { _u6 = __u6; };

  Union7 * u7 () { return new Union7(_u7); };
  void u7 (const Union7 & __u7) { _u7 = __u7; };
};

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  unions_impl * pt = new unions_impl;
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
