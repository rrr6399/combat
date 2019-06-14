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
# Connect to the server. Its reference is in ./server.ior
#

set obj [corba::string_to_object file://[pwd]/server.ior]

#
# Say Hello World!
#

$obj hello
