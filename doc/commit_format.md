optimize code.

Description:
producer have a bug, when we start producer, we didn't check if
it is ready for work. we app exit before prouder is ready, there
may be coredump.

Major Changes:
1. add sleep in test_stub_add_hooks, for the app exited before producer is ready.
2. renamed:    src/concurrent/worker.c -> src/concurrent/api.c
   renamed:    src/net/worker/client_compat.c -> src/net/worker/client_api.c
   renamed:    src/net/worker/server_compat.c -> src/net/worker/server_api.c