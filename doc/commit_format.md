[update:bus] 修复bus RB太小导致tcp数据合并后有可能丢失问题。

Description:
tcp数据合并后超过了RB大小， 导致后续解析数据错误。

Major Changes:
1. 修复bus bug.