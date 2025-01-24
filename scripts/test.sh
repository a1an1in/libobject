#! /bin/bash
pid=$(ps -aux|grep "test-process" | grep -v grep | awk '{print $2}')

kill -n 30 $pid

