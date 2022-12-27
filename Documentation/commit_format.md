change libobject init and destroy funcs.

Description:
we change the init and destroy funcs position and del useless files.

Major Changes:
1. del init.c and init.h of core.
2. some modules may need init, we do those process in argument module.