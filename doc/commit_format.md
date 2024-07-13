[bugfix:NA] 修复node在linux运行错误。

Description:
之前node是在windows环境开发的， 还没在linux运行过，
修复linux运行bug。

Major Changes:
1. 修复Fshell没有包含stub.h导致获取的地址是32bit的，导致段错误。
2. 修复Openssl::Digest_HmacSha1, 替换::为_。