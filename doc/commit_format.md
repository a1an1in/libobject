add some debug info for Event Tread.

Description:
there's some issue on windows for Event Tread, which
may not able to receive exit signal at some cases.

Major Changes:
1. del event for evsig_release
2. change vip debug level
3. add dbg log for concurrent.
