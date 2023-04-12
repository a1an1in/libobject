#!/usr/bin/expect

# Script to pass automatically the
# password when requested.
#
# Usage:
# expect.sh <password> <commands_that_will_require_password>

#Uncomment the following in order to debug the script
#exp_internal 1

set cmd [lindex $argv 1]
set cmd_with_args [lrange $argv 1 end]
set password [lindex $argv 0]
set interact_flag false

send_user "argc: $argc, argv: $argv \r\n"

if { "$cmd" == "scp" } {
    # Currently scp to deploy an image takes around 7 minutes
    set timeout 600
} elseif { "$cmd" == "ssh" && $argc == 3 } {
    send_user "ssh login, don't set timeout \r\n"
    set timeout -1
}

eval spawn $cmd_with_args
expect {
    -re "\[P|p]assword:" { exp_send "$password\r"
                           exp_continue }
    "yes/no"             { exp_send "yes\r"; interact }
    timeout              { send_error "\r\nError: Waiting time ($timeout sec) for $cmd expired\r\n"
                           exit 1 }
    "login"              { interact }
    eof
}

set cmd_ret [wait]
if { [lindex $cmd_ret 2] != 0 } {
    send_error "return value:$cmd_ret"
    send_error "System error occurred, errno=[lindex $cmd_ret 2]\r\n"
    exit [lindex $cmd_ret 2]
} elseif { [lindex $cmd_ret 3] != 0 } {
    send_error "return value:$cmd_ret"
    send_error "Error: $cmd exit code [lindex $cmd_ret 3]\r\n"
    exit [lindex $cmd_ret 3]
} else {
    exit 0
}
