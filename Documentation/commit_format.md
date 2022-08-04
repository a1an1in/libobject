refact Number Class.

Description:
the implement policies of set_value and get_value have some issue. there's no
need using policies, we can do it directly.

Major Changes:
1. change Number set_value, get_value interface, add a len parameter.
1. change Number add().
2. implement big number
