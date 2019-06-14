#include "server.h"
#include <fstream>

using namespace std;

class composed_impl : virtual public POA_composed
{
private:
  Enum1 _e1;
  Enum2 _e2;
  datum _d;
  Struct1 _s1;
  Struct2 _s2;
  Struct3 _s3;
  Struct4 _s4;
  Seq1 _q1;
  Seq2 _q2;
  Seq3 _q3;
  Seq4 _q4;
  Arr1_var _a1;
  Arr2_var _a2;
  Arr3_var _a3;
  Arr4_var _a4;
  OctSeq _os;

public:
  Enum1 e1 () { return _e1; };
  void e1 (Enum1 __e1) { _e1 = __e1; };

  Enum2 e2 () { return _e2; };
  void e2 (Enum2 __e2) { _e2 = __e2; };

  datum d () { return _d; };
  void d (datum __d) { _d = __d; };

  Struct1 * s1 () { return new Struct1(_s1); };
  void s1 (const Struct1 & __s1) { _s1 = __s1; };

  Struct2 * s2 () { return new Struct2(_s2); };
  void s2 (const Struct2 & __s2) { _s2 = __s2; };

  Struct3 * s3 () { return new Struct3(_s3); };
  void s3 (const Struct3 & __s3) { _s3 = __s3; };

  Struct4 * s4 () { return new Struct4(_s4); };
  void s4 (const Struct4 & __s4) { _s4 = __s4; };

  Seq1 * q1 () { return new Seq1(_q1); };
  void q1 (const Seq1 & __q1) { _q1 = __q1; };

  Seq2 * q2 () { return new Seq2(_q2); };
  void q2 (const Seq2 & __q2) { _q2 = __q2; };

  Seq3 * q3 () { return new Seq3(_q3); };
  void q3 (const Seq3 & __q3) { _q3 = __q3; };

  Seq4 * q4 () { return new Seq4(_q4); };
  void q4 (const Seq4 & __q4) { _q4 = __q4; };

  Arr1_slice * a1 () { return Arr1_dup (_a1.in()); };
  void a1 (const Arr1 __a1) { _a1 = Arr1_dup (__a1); };

  Arr2_slice * a2 () { return Arr2_dup (_a2.in()); };
  void a2 (const Arr2 __a2) { _a2 = Arr2_dup (__a2); };

  Arr3_slice * a3 () { return Arr3_dup (_a3.in()); };
  void a3 (const Arr3 __a3) { _a3 = Arr3_dup (__a3); };

  Arr4_slice * a4 () { return Arr4_dup (_a4.in()); };
  void a4 (const Arr4 __a4) { _a4 = Arr4_dup (__a4); };

  OctSeq * os () { return new OctSeq(_os); };
  void os (const OctSeq & __os) { _os = __os; }
};

int main (int argc, char *argv[])
{
  CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
  CORBA::Object_var poaobj = orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poaobj);
  PortableServer::POAManager_var mgr = poa->the_POAManager();

  composed_impl * pt = new composed_impl;
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
