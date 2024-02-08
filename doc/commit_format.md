[update:bus] optimize bus.

Description:
add cname to bus object, then we can search object by
cname afterwards.

Major Changes:
1. fix lookup_sync and invoke_sync.
2. add cname for bus object.