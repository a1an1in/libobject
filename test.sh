#! /bin/bash
pid=$(ps -el|grep "mockery" | grep -v grep | awk '{print $2}')

kill -n 30 $pid

