# Tcl package index file, version 1.1
# This file is generated by the "pkg_mkIndex" command
# and sourced either when an application starts up or
# by a "package unknown" script.  It invokes the
# "package ifneeded" command to set up package-related
# information so that packages will be loaded automatically
# in response to "package require" commands.  When this
# script is sourced, the variable $dir must contain the
# full path name of this file's directory.
# Hacked to work on Win32 and Unix

global tcl_platform
if {$tcl_platform(platform) == "windows"} {
  package ifneeded combat 0.7 [list load [file join $dir combat.dll]]
} else {
  package ifneeded combat 0.7 [list load [file join $dir libcombat[info sharedlibextension]]]
}
