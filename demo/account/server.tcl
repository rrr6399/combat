#! /bin/sh
# \
exec combatsh "$0" ${1+"$@"}

#
# require Itcl
#

package require Itcl

#
# The Account server
#

class Account_impl {
    inherit PortableServer::ServantBase

    public method _Interface {} {
	return "IDL:Account:1.0"
    }

    private variable balance 0

    public method deposit { amount } {
	set balance [expr $balance + $amount]
    }

    public method withdraw { amount } {
	if {$amount > $balance} {
	    corba::throw [list IDL:Account/Bankrupt:1.0 \
		    [list balance $balance amount $amount]]
	}
	set balance [expr $balance - $amount]
    }

    public method balance {} {
	return $balance
    }

    public method destroy {} {
	set obj [corba::resolve_initial_references POACurrent]
	set poa [$obj get_POA]
	set oid [$obj get_object_id]
	$poa deactivate_object $oid
    }
}

class Bank_impl {
    inherit PortableServer::ServantBase

    public method _Interface {} {
	return "IDL:Bank:1.0"
    }

    public variable id 0

    public method create {} {
	global mypoa
	incr id
	return [$mypoa create_reference_with_id Account_$id IDL:Account:1.0]
    }
}

#
# The ServantActivator
#

class Activator {
    inherit PortableServer::ServantBase

    public method _Interface {} {
	return "IDL:omg.org/PortableServer/ServantActivator:1.0"
    }

    #
    # The new account object must be created on global level
    #

    public method incarnate { oid adapter } {
	puts "Incarnating account $oid"
        return [namespace current]::[Account_impl #auto]
    }

    public method etherealize { oid adapter serv cleanup remaining } {
	if {!$remaining} {
	    puts "Etherealizing account $oid"
	    delete object $serv
	}
    }
}

#
# Initialize ORB and feed the local Interface Repository
#

eval corba::init $argv
source test.tcl
combat::ir add $_ir_test

#
# Create a new POA with the USE_SERVANT_MANAGER policy
#

set poa   [corba::resolve_initial_references RootPOA]
set mgr   [$poa the_POAManager]
set mypoa [$poa create_POA MyPOA $mgr {RETAIN USE_SERVANT_MANAGER USER_ID}]

#
# Register our ServantManager, which will incarnate
# new accounts on request
#

set amgr [Activator #auto]
set amgr_ref [$amgr _this]
$mypoa set_servant_manager $amgr_ref

#
# Create a Bank and activate it
#

set srv [Bank_impl #auto]
set oid [$poa activate_object $srv]

#
# write Bank's IOR to file
#

set reffile [open "server.ior" w]
set ref [$poa id_to_reference $oid]
set str [corba::object_to_string $ref]
puts -nonewline $reffile $str
close $reffile

#
# Activate the POAs
#

$mgr activate

#
# .. and serve the bank ...
#

puts "Running."
vwait forever

puts "oops"
