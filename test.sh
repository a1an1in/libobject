#! /bin/bash
pid=$(ps -el|grep "test mockery" | grep -v grep | awk '{print $2}')

kill -n 30 $pid

