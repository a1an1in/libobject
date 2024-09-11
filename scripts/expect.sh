#!/usr/bin/expect
 
# Script to pass automatically the
# password when requested.
#
# Usage:
# tca_login_passwd_exp <password> <commands_that_will_require_password>
 
#Uncomment the following in order to debug the script
#exp_internal 1
 
set cmd [lindex $argv 1]
set cmd_with_args [lrange $argv 1 end]
set password [lindex $argv 0]
 
if { "$cmd" == "scp" || "$cmd" == "ssh" } {
    # Currently scp to deploy an image takes around 7 minutes
    set timeout 1200
}
# send_error "spawn $argv\r"
eval spawn $cmd_with_args
expect {
    -re "\[P|p]assword:"      { exp_send "$password\r";exp_continue }
    "(yes/no"                 { send "yes\r";exp_continue }
    timeout                   { send_error "\r\nError: Waiting time ($timeout sec) for $cmd expired\r\n";exit 1 }
    eof
}
 
set cmd_ret [wait]
 
if { [lindex $cmd_ret 2] == 0 } {
    if { [lindex $cmd_ret 3] != 0 } {
        # send_error "Error: $cmd exit code [lindex $cmd_ret 3]\r\n"
    }
} else {
    send_error "System error occurred, errno=[lindex $cmd_ret 3]\r\n"
}
 
exit [lindex $cmd_ret 3]
 