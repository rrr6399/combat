#! /bin/sh
# \
exec combatsh "$0" ${1+"$@"}

#
# Initialize ORB and feed the local Interface Repository
#

eval corba::init $argv
source test.tcl
combat::ir add $_ir_test

#
# Connect to the Bank. Its reference is in ./server.ior
#

set Bank [corba::string_to_object file://[pwd]/server.ior]

#
# Open an account
#

set Account [$Bank create]

#
# Deposit and Withdraw some money
#

$Account deposit 700
$Account withdraw 450

#
# Ba-Ba-Bankrobbery! (Erste Allgemeine Verunsicherung, ca. 1985)
#

set ok 1

corba::try {
  $Account withdraw 1000000
} catch {IDL:Account/Bankrupt:1.0} {
  #
  # damn, couldn't get away with it
  #
  set ok 0
} catch {... oops} {
  #
  # oops, what's up, Doc?
  #
  error "Unexpected exception: $oops"
}

if {$ok} {
    puts "successfully withdrew a million bucks!"
}

#
# Look what's left
#

puts "Balance is [$Account balance]."

#
# Finalize and exit
#

$Account destroy
