unify stub interface and make it runnable on windows.

Description:
After a few iteration, the lib can't run on windows. this
commit is to solve this windows issues and debuged stub
feature. This lib can run windows now.

Major Changes:
1. let this lib can complie on windows.
2. solve compiling errors on windows.
3. refact stub_parse_context func, del some paramaters.
4. the stub has passed tests on windows.