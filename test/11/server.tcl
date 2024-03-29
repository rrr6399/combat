#! /bin/sh
# \
if test -f ../../combatsh ; then exec ../../combatsh "$0" ${1+"$@"} ; fi
# \
exec tclsh8.3 "$0" ${1+"$@"}

if {[file exists ../../combat.tcl]} {
    lappend auto_path ../..
    package require combat
}

#
# The Server
#

class DiamondA_impl {
    inherit PortableServer::ServantBase
    public method _Interface {} {
	return "IDL:diamonda:1.0"
    }
    public method opa {} {
	return "opa"
    }
}

class DiamondB_impl {
    inherit DiamondA_impl
    public method _Interface {} {
	return "IDL:diamondb:1.0"
    }
    public method opb {} {
	return "opb"
    }
}

class DiamondC_impl {
    inherit DiamondA_impl
    public method _Interface {} {
	return "IDL:diamondc:1.0"
    }
    public method opc {} {
	return "opc"
    }
}

#
# must avoid diamond inheritance in [incr Tcl]. We can use delegation, or
# just re-implement everything. Here, we inherit diamondb and have to add
# the implementation for diamondc on our own.
#

class DiamondD_impl {
    inherit DiamondB_impl
    public method _Interface {} {
	return "IDL:diamondd:1.0"
    }
    public method opc {} {
	return "opc"
    }
    public method opd {} {
	return "opd"
    }
}

class Server_impl {
    inherit PortableServer::ServantBase

    public method _Interface {} {
	return "IDL:operations:1.0"
    }

    public variable s 42
    public variable ra "Hello World"

    public method square { x } {
	return [expr {$x * $x}]
    }

    public method copy { sin sout_name } {
	upvar $sout_name sout
	set sout $sin
	return [string length $sin]
    }

    public method length { queue oe_name } {
	upvar $oe_name oe
	set res [llength $queue]
	if {[expr $res % 2] == 0} {
	    set oe EVEN
	} else {
	    set oe ODD
	}
	return $res
    }

    public method squares { howmany } {
	set res ""
	for {set i 0} {$i < $howmany} {incr i} {
	    lappend res [list member [expr $i * $i]]
	}
	return $res
    }

    public method reverse { str_name } {
	upvar $str_name str
	set res ""
	foreach c [split $str {}] {
	    set res $c$res
	}
	set str $res
    }

    public method nop {} {
    }

    public method dup {} {
	return [_this]
    }

    public method dup2 {o1 o2_name} {
	upvar $o2_name o2
	set o2 $o1
    }

    public method isme {obj} {
	return [[_this] _is_equivalent $obj]
    }

    public method getdiamond {} {
	set da [namespace current]::[DiamondA_impl #auto]
	set db [namespace current]::[DiamondB_impl #auto]
	set dc [namespace current]::[DiamondC_impl #auto]
	set dd [namespace current]::[DiamondD_impl #auto]

	#
	# According to the CORBA 2.3 specs, implicit activation doesn't
	# work for DSI servants. This is probably a bug in the specs and
	# has been reported as an issue.
	#

	set res(a) [$::poa servant_to_reference $da]
	set res(b) [$::poa servant_to_reference $db]
	set res(c) [$::poa servant_to_reference $dc]
	set res(d) [$::poa servant_to_reference $dd]
	set res(abcd) [list $res(a) $res(b) $res(c) $res(d)]

	return [array get res]
    }

    public method DontCallMe {} {
	corba::throw {IDL:Oops:1.0 {what {I said, don't call me!}}}
    }
}

#
# Initialize ORB
#

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
