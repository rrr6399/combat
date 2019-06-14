#! /bin/sh
# \
exec combatsh "$0" ${1+"$@"}

#
# require Itcl
#

package require Itcl

#
# Hello server implementation
#

class HelloWorld {
    inherit PortableServer::ServantBase
    
    public method _Interface {} {
	return "IDL:HelloWorld:1.0"
    }
  
    public method hello {} {
	puts "Hello World"
    }
}

#
# Initialize ORB and feed the local Interface Repository
#

eval corba::init $argv
source test.tcl
combat::ir add $_ir_test

#
# Obtain a POA pseudo object and retrieve its Manager
#

set poa [corba::resolve_initial_references RootPOA]
set mgr [$poa the_POAManager]

#
# Create a HelloWorld Server and activate it
#

set srv [HelloWorld #auto]
set oid [$poa activate_object $srv]

#
# write IOR to file
#

set reffile [open "server.ior" w]
set ref [$poa id_to_reference $oid]
set str [corba::object_to_string $ref]
puts -nonewline $reffile $str
close $reffile

#
# Activate the POA
#

$mgr activate

#
# ... and serve ...
#

puts "Running."
vwait forever

puts "oops"
