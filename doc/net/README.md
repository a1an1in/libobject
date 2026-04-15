# Net Module

The net module provides networking capabilities, including:

- **TCP/UDP Socket Programming**: Create and manage network connections.
- **HTTP Support**: Send and receive HTTP requests.

## http
### test commands
```
./sysroot/linux/x86_64/bin/xtools httpd --log-level=0x16 -h "0.0.0.0" -s "8081"
curl -ksX GET https://www.yunisona.top:51080/tangerine/api/hello_world
curl -ksX GET  https://119.4.206.26:51080/tangerine/api/http_plugin_test
curl -X GET 127.0.0.1:8081/api/hello_world
curl -X GET 127.0.0.1:8081/api/http_plugin_test


./sysroot/linux/x86_64/bin/xtools httpd --log-level=0x16 -h "0.0.0.0" -s "8081"
http://127.0.0.1:8081/test2.mp4

sudo tcpdump -i any -w http_8081.pcap 'port 8081'
./sysroot/linux/x86_64/bin/xtools --log-level=0x16 mockery -f test_http_mp4
New-NetFirewallRule -DisplayName "Allow 8081 Port For xtools" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 8081 -Profile Domain,Private,Public
```