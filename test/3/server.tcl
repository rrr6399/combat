#! /bin/sh
# \
if test -f ../../combatsh ; then exec ../../combatsh "$0" ${1+"$@"} ; fi
# \
exec tclsh8.3 "$0" ${1+"$@"}

if {[file exists ../../combat.tcl]} {
    lappend auto_path ../..
    package require combat
}

class Server_impl {
    public method _Interface {} {
	return "IDL:unions:1.0"
    }
    public variable u1
    public variable u2
    public variable u3
    public variable u4
    public variable u5
    public variable u6
    public variable u7
}

source test.tcl
eval corba::init $argv
combat::ir add $_ir_test

#
# Create a Server server and activate it
#

set poa [corba::resolve_initial_references RootPOA]
set mgr [$poa the_POAManager]
set srv [Server_impl #auto]
set oid [$poa activate_object $srv]

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
# .. and start serving requests ...
#

vwait forever

puts "oops"
