[update:encoding] 去掉openssl依赖。

Description:
Openssl_Digest_HmacSha1使用不多而且依赖openssl，导致
编译很慢，所以先暂时去掉该类的实现。

Major Changes:
1. 去掉openssl依赖。