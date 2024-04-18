[update:EventBase] 修复EB问题.

Description:
修复event base问题， 删掉的fd找不到时去执行了上次运行的event.

Major Changes:
1. Eventbase 在activate_io有bug， 找不到io也会去执行ev_callback，
   只不过有可能是上次的callback，所有会导致程序卡住。
2. 修改部分模块关键的日志的等级。
3. 修改test_net, 添加等待时间。