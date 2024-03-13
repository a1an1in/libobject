[update:concurrent] 移动message test case到test_messsage.c file.

Description:
message的test和源码放在一块的， 这里把test移到test文件。 测试过程
中发现test_message_publisher， 原因是Centor使用io_worker但是
没有包含worker_api.h， 使用的隐含声明， 所有返回的地址大小不对。

Major Changes:
1. 移动test cases。
2. 修复message coredump.