bugfix skcipher.

Description:
at mac platform, regex may not support '\w' pattern, i relace it with
[a-z0-9A-Z]+. then we can get algo mode and sub algo now

Major Changes:
1. fix skipher set algo error in mac platform