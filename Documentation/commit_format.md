To optimize worker, and fix a issue in fshell

Description:
update.

Major Changes:
1.del io_worker.c, merge signal, timer and io worker into worker.c
2.add try catch for event_del
3.change event_assign to signal_worker.
4.mv test of worker to test_worker.c