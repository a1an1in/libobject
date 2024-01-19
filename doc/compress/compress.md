# 压缩

gzip是一种文件压缩工具（或该压缩工具产生的压缩文件格式），它的设计目标是处理单个的文件。gzip在压缩文件中的数据时使用的就是zlib。为了保存与文件属性有关的信息，gzip需要在压缩文件（*.gz）中保存更多的头信息内容，而zlib不用考虑这一点。但gzip只适用于单个文件，所以我们在UNIX/Linux上经常看到的压缩包后缀都是*.tar.gz或*.tgz，也就是先用tar把多个文件打包成单个文件，再用gzip压缩的结果。

[zlib](https://baike.baidu.com/item/zlib?fromModule=lemma_inlink)是DEFLATE算法的实现库，它的API同时支持gzip文件格式以及一个简化的数据流格式。zlib数据流格式、DEFLATE以及gzip文件格式均已被分别标准化为RFC 1950、RFC 1951、RFC 1952。

deflate(RFC1951):一种压缩算法，使用LZ77和哈弗曼进行编码；
zlib(RFC1950):一种格式，是对deflate进行了简单的封装，他也是一个实现库(delphi中有zlib,zlibex)
gzip(RFC1952):一种格式，也是对deflate进行的封装。

gzip = gzip头 + deflate编码的实际内容 + gzip尾
zlib = zlib头 + deflate编码的实际内容 + zlib尾

## zlib

1. deflateInit2 

```cpp
z_streamp strm: z_stream 指针；

int level: 压缩等级，必需为 Z_DEFAULT_COMPRESSION 或者 0 ~ 9 的整数，1为最快，9为最大限度压缩，0为不压缩，数字越大越耗时；

int method: 压缩算法，只支持 Z_DEFLATED；

int windowBits: 历史缓冲区最大尺寸，值为 2^windowBits, windowBits 的值为 8~15 时，deflate() 方法生成 zlib 格式的数据，当 windowBits 为 31 时 deflate() 方法生成 gzip 格式。当取值为 -15 ~ -8 时，deflate() 生成纯 deflate 算法压缩数据（不包含 zlib 和 gzip 格式头和尾）， compress 函数里面使用的windowBits 是15， 所以这个函数是用于zlib压缩的函数。

int strategy: 用于调整压缩算法，一般使用 Z_DEFAULT_STRATEGY
```

2. deflate(z_streamp strm, int flush) 

flsuh参数：

Z_NO_FLUSH：通常设置为该值，允许压缩算法决定累积多少数据再产生输出，以达到压缩效率最高。

Z_SYNC_FLUSH：将所有等待输出的数据刷新到输出缓冲区，以字节为边界进行对 齐。该刷新可能会降低压缩算法的压缩效率，它只用于必要的时候。This completes the current deflate block and follows it with an empty stored block that is three bits plus filler bits to the next byte, followed by four bytes (00 00 ff ff).

Z_FINISH：如果输入和待输出的数据都被处理完，则返回 Z_STREAM_END。如果返 回 Z_OK or Z_BUF_ERROR 则需要再次调用 Z_FINISH 直到返回 Z_STREAM_END

## tar

```
tar  -zxvf  wwwdata.tar.gz   ./xxx  解压缩指定文件
tar -zxvf archive.tar.bz2 --wildcards "file*" 解压缩匹配文件
tar -rvf archive.tar file-new*将文件添加到存档
tar --delete -f archive.tar file-new.txt file-new2.txt 从存档中删除文件
tar -dvf archive.tar file4.txt 比较文件和文档差异
```

### 源码分析

open_archive

-->sys_child_open_for_uncompress

​	-->run_decompress_program

### tar包的结构

```
文件头 – 文件内容 – 文件头 – 文件内容 ------ 文件末尾

type Header struct {
	name     [100]byte
	mode     [8]byte
	owner    [8]byte
	group    [8]byte
	size     [12]byte
	mtime    [12]byte
	checkSum [8]byte
	fileType byte
	linkName [100]byte
	magic	 [6]byte
　　version	 [2]byte
　　uname	 [32]byte
　　gname	 [32]byte
　　devmajor [8]byte
　　devminor [8]byte
　　prefix   [155]byte
　　padding  [12]byte
}
以上是Tar中保存文件信息的数据结构，其后跟着的就是文件的内容。

其中，文件大小，修改时间，checksum都是存储的对应的八进制字符串，字符串最后一个字符为空格字符checksum的计算方法为出去checksum字段其他所有的512-8共504个字节的Ascii码相加的值再加上256文件内容以512字节为一个块进行分割，最后一个块不足部分以0补齐多个文件合成的tar，存储格式为：tar的头结构，文件体，tar头，文件体……。当所有文件都存储完成后，在文件末尾补上一个全零的tar结构（即1024个零值）所有的tar文件大小都是512的倍数一个空的文件，打包成tar后，为512*3个字节
```

 ![image-20231109171540546](C:\Users\86158\AppData\Roaming\Typora\typora-user-images\image-20231109171540546.png)

## zip

byte_before_the_zipfile 做什么用的？

file header

时间处理：

zip64local_TmzDateToDosDate



refs:

[压缩包Zip格式详析](https://blog.csdn.net/qq_43278826/article/details/118436116)

[使用zlib解压标准zip文件](https://blog.csdn.net/wishfly/article/details/46408595)

[zlib库介绍四：zlib算法（LZ77、LZ78、霍夫曼编码、滑动窗口、Rabin-Karp算法、哈希链、I/O缓冲区）](https://blog.csdn.net/LuckyHanMo/article/details/124781819)

[使用minizip解压缩多个文件（基于zlib）](https://blog.csdn.net/whahu1989/article/details/80344373)****

[pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT](https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT)

[ZIP压缩算法详细分析及解压实例解释 - esingchan - 博客园 (cnblogs.com)](https://www.cnblogs.com/esingchan/p/3958962.html)

## rar

源码路径：http://www.rarsoft.com/rar/rarlinux-x64-5.4.0.tar.gz

## refs

[深入理解gzip原理](https://www.jianshu.com/p/4033028e5570)

[zlib的解压与压缩代码](https://blog.csdn.net/qq_46074683/article/details/107080803)

[使用zlib实现gzip压缩](https://blog.csdn.net/u014608280/article/details/115136125)

https://blog.csdn.net/chary8088/article/details/48047835/

[Linux(程序设计):28---数据流压缩原理（Deflate压缩算法、gzip、zlib)](https://blog.csdn.net/qq_41453285/article/details/106685915)

[tar 命令](https://blog.51cto.com/waleon/5525496)

[压缩 / 解压（tar,rar,zip,7z... 分卷压缩...）](https://blog.csdn.net/lovechris00/article/details/130701086)

[文件格式](https://www.fileformat.info/format/all.htm)

https://blog.csdn.net/m0_63593710/article/details/13001939

[无损数据压缩算法的历史](https://blog.csdn.net/kimylrong/article/details/39405981)

