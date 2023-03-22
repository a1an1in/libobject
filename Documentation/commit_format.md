fix issue with client_compat.c file.

Description:
trustee net/Client before connecting, there will be "Transport endpoint 
is not connected" issue. so we have to implement trustee compat interface
to fix it. we don't truestee callback at client func.

Major Changes:
1. fix "Transport endpoint is not connected" issue.
2. fix some compling warnnings.