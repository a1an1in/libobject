add windows pipe

Description:
i plan to replace concurrent socket, which use 2 port and
must config port when run at xtools. after search we found
that on windows select can only be used for socket, so it
cann't replaced.

Major Changes:
1. add windows pipe.
