package require Itcl
package require http

####################################################################
## The most general interface: send act/dev/msg to server, get answer.
##
## Usage:
##  set Device2::addr http://localhost:8082
##  Device2::get list {} {}
##  Device2::get ask graphene get_time
##  ...
##  or
##  Device2::list
##  Device2::ask graphene get_time

namespace eval Device2 {
  # Server address. Should we read it from /etc/device_c.cfg?
  set addr "http://localhost:8082"

  # nutmeat
  proc get {act dev args} {
    set act [http::quoteString $act]
    set dev [http::quoteString $dev]
    set msg [http::quoteString $args]
    set url "$Device2::addr/$act/$dev/$msg"
    set token [::http::geturl $url -keepalive true]
    set code [::http::ncode $token]
    set err  [::http::error $token]
    set data [::http::data $token]
    if {$code != 200} {error $data}
    return $data
  }

  # syntactic sugar
  proc ask      {dev args} {Device2::get ask $dev {*}$args}
  proc list     {}    {Device2::get list     {} {}}
  proc reload   {}    {Device2::get reload   {} {}}
  proc ping     {}    {Device2::get ping     {} {}}
  proc get_time {}    {Device2::get get_time {} {}}
  proc info     {dev} {Device2::get info    $dev {}}
  proc use      {dev} {Device2::get use     $dev {}}
  proc release  {dev} {Device2::get release $dev {}}
  proc lock     {dev} {Device2::get lock    $dev {}}
  proc unlock   {dev} {Device2::get unlock  $dev {}}
  proc log_start  {dev} {Device2::get log_start  $dev {}}
  proc log_finish {dev} {Device2::get log_finish $dev {}}
  proc log_get    {dev} {Device2::get log_get    $dev {}}
}

####################################################################
## Simulator of old good Device (not exact)
##
## Usage:
##  Device lockin0
##  lockin0 cmd *IDN?
##  DeviceDelete lockin0

itcl::class Device {
  variable name;   # device name

  # not used, should we leave it for compatibility?
  public common sync 0;

  constructor {} {
    # get device name (remove tcl namespace from $this)
    set name $this
    set cn [string last ":" $name]
    if {$cn >0} {set name [string range $name [expr $cn+1] end]}

    # Open end test
    Device2::use $name
  }
  destructor {
    Device2::release $name
  }

  method cmd {args} { Device2::get ask $name {*}$args }
  method lock   {} { Device2::get lock $name {} }
  method unlock {} { Device2::get unlock $name {} }
}

# If device was created in a namespace, use [namespace current]::$name
# in DeviceExists and DeviceDelete argumants!

# Check if <name> is a Device object
proc DeviceExists {name} {
  if {[catch {set base [lindex [$name info heritage] end]} ]} { return 0 }
  return [expr {$base == {::Device}}]
}

# Delete device object <name>
proc DeviceDelete {name} {
  if {![DeviceExists $name]} { error "Can't close non-existing Device: $name" }
  itcl::delete object $name
}

