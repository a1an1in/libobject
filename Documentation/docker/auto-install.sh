#!/usr/bin/env expect
set timeout -1
set installer [lindex $argv 0]
set install_path [lindex $argv 1]

spawn $installer --dir $install_path
expect "Press Enter to display the license agreements"
send "\n"
send "q"
expect "*Do you accept Xilinx End User License Agreemen*"
send "y\n"
send "q"
expect "*Do you accept Third Party End User License Agreement*"
send "y\n"
expect eof
catch wait result
exit [lindex $result 3]
