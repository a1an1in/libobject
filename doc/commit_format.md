[bugfix:attacher] fix sprintf break issue.

Description:
The rsp register have some issue when the func have
no args. after that, the attacher can call stub func
now.

Major Changes:
1. fix sprintf break issue
2. fix a comma issue.
3. add attach stub test.
