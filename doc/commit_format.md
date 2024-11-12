[update:bus] 新增ping/pong for bus。

Description:
bus新增保活协议，tcp自带的keeplive不能保证server是活的，
即使busd认为object service掉线了， client也能收到keeplive
应答，所以一直不能访问service。

Major Changes:
1. 新增ping/pong协议。