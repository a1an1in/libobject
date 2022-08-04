refact Number Class.

Description:
the old version of Number is not extensible. we use a g_number_policies to adapt
a sort of situation.

Major Changes:
1. change Number set_value, get_value interface, add a len parameter.
1. change Number add().
2. implement big number
