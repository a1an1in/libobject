# archive 模块

归档（打包 / 解包）模块。统一抽象基类 `Archive` + 6 种格式实现 + 通用打开命令 `Archive_Command`，
二进制格式说明见本目录下的 [`7z_format.txt`](7z_format.txt)、[`squashfs.txt`](squashfs.txt)、[`tar.txt`](tar.txt)、[`zip_format.txt`](zip_format.txt)。

> **通用"打开任意归档"命令 `Archive_Command`（CLI 命令 `archive`）已实现**：按文件魔数自动识别格式（含 squashfs / zip / 7z / tar / tgz / tbz2）。
> 命令行用法见 [`archive_command.md`](archive_command.md)，设计见下文[「六、通用打开任意归档命令」](#六通用打开任意归档命令archive_command)。

## 一、目录结构

```
src/archive/
├── Archive.c              # 统一抽象基类（open/close/list/extract/add/save/通配符过滤）
├── Archive_Command.c/.h   # 通用 CLI 命令 archive：魔数识别 + 分发 + list/extract/create/add
├── CMakeLists.txt
├── tar/    Tar.c          # tar 归档
├── zip/    Zip.c          # zip 归档（STORED/DEFLATED、CRC32）
├── tgz/    Tgz.c          # tar + gzip
├── tbz2/   Tbz2.c         # tar + bzip2
├── 7z/     SevenZip.c     # 7z 归档（LZMA/LZMA2、固实压缩）
└── squashfs/ Squashfs.c   # SquashFS 4.0 镜像（完整读写）

tests/archive/
├── test_tar.c / test_tgz.c / test_tbz2.c
├── test_zip.c             # 含系统 unzip 互操作校验
├── test_7z.c              # 含与其他 7z 工具互操作
├── test_squashfs.c        # 含 unsquashfs/mksquashfs 双向验证
└── test_archive_cli.c     # Archive_Command CLI 端到端测试(popen 跑 xtools archive, 按格式校验)
```

## 二、模块概况

| 格式 | 实现文件 | 代码量 | 能力 |
| --- | --- | --- | --- |
| Tar | `src/archive/tar/Tar.c` | 265 行 | list / extract / add / save |
| Zip | `src/archive/zip/Zip.c` | 729 行 | list / extract / add / save，STORED + DEFLATED，CRC32，正则过滤提取 |
| Tgz | `src/archive/tgz/Tgz.c` | 102 行 | tar + gzip |
| Tbz2 | `src/archive/tbz2/Tbz2.c` | 113 行 | tar + bzip2 |
| 7z | `src/archive/7z/SevenZip.c` | 1345 行 | LZMA/LZMA2、固实压缩 folder/substream、加密头解析 |
| Squashfs | `src/archive/squashfs/Squashfs.c` | 1197 行 | SquashFS 4.0 完整读写（superblock/inode/多级目录树/fragment/id 表） |

## 三、实现得比较全面的地方

1. **格式覆盖广，且基本是"读 + 写"双向支持**
   6 种格式均实现 list / extract / add / save 完整链路，而非只读或只写。

2. **统一抽象基类设计良好**
   `Archive.c` 提供 open/close、list、extract（含 extract_file / extract_files）、
   add（含递归目录树 `fs_tree` 遍历）、save、包含/排除通配符过滤、compress/uncompress 等一致接口，
   6 种格式均为其子类，扩展新格式成本低。

3. **与外部工具互操作，兼容性有保障**
   - `test_zip`：用系统 `unzip` 校验生成的归档
   - `test_tbz2`：与系统 `bzip2` 互解
   - `test_squashfs`：与 `unsquashfs` / `mksquashfs` 双向验证

4. **实现深度足够**
   7z 的固实压缩 folder/substream 划分、Squashfs 的完整元数据表解析，均非"能用就行"的浅实现。

## 四、仍可继续完善的方向

1. **压缩算法覆盖有限**
   - Squashfs 目前仅支持 zlib（`SQFS_COMPRESSION_ZLIB=1`），常见 SquashFS 默认的 xz / lzo / lz4 / zstd 未实现
   - Zip 仅 STORED / DEFLATED，无 DEFLATE64 / bzip2 / LZMA / ZSTD 方法

2. **元数据保真不足**
   - Tar 添加时 `mode` 写死 777，未保留 uid/gid/符号链接
   - Squashfs 写端 uid/gid 索引写死为 0，symlink 类型未处理，xattr 明确不支持（`SQFS_FLAG_NOXATTR`）

3. **加密能力缺失**
   - Zip 无传统 / AES 加密；7z 的加密头更多是"能跳过解析"

4. **大文件限制**
   - `archive_file_info_t.size` 及 Zip 内部 size 均为 `uint32_t`，>4GB 文件存在截断风险

5. **工程化能力**
   - 暂无进度 / 取消回调、多线程压缩、分卷支持

## 五、常见归档 / 压缩格式总览

按"归档容器"（打包多个文件）与"压缩流"（压缩单个数据）两类划分。
魔数是文件开头的固定字节，是自动识别格式的关键，也是[第六节](#六通用打开任意归档命令archive_command)通用打开命令的依据。

| 格式 | 文件头魔数 | 本项目支持 | 说明 |
| --- | --- | --- | --- |
| Tar | offset 257: `75 73 74 61 72`（`ustar`） | ✅ `Tar` | 只打包不压缩，Linux 标准 |
| Tgz（tar.gz） | `1f 8b`（gzip 流） | ✅ `Tgz` | tar + gzip |
| Tbz2（tar.bz2） | `42 5a 68`（`BZh`） | ✅ `Tbz2` | tar + bzip2 |
| Zip | `50 4b 03 04`（`PK\x03\x04`），空包 `50 4b 05 06` | ✅ `Zip` | STORED / DEFLATED |
| 7z | `37 7a bc af 27 1c`（`7z\xBC\xAF\x27\x1C`） | ✅ `SevenZip` | LZMA / LZMA2 |
| Squashfs | `68 73 71 73`（`hsqs`） | ✅ `Squashfs` | 只读压缩文件系统镜像，当前仅 zlib |
| gzip 单文件 | `1f 8b` | 🟡 仅 `Compress` 模块 | 非归档容器 |
| bzip2 单文件 | `42 5a 68` | 🟡 仅 `Compress` 模块 | 非归档容器 |
| Xz（.tar.xz） | `fd 37 7a 58 5a 00` | ❌ | 需新增 xz 流式解压 |
| Zstd（.tar.zst） | `28 b5 2f fd` | ❌ | 需新增 |
| Lz4 | `04 22 4d 18` | ❌ | 需新增 |
| RAR | `52 61 72 21 1a 07`（`Rar!`） | ❌ | 以解压为主（RAR 压缩算法闭源） |
| ISO9660 | offset 0x8001: `43 44 30 30 31`（`CD001`） | ❌ | 只读光盘镜像 |
| Cpio | `30 37 30 37`（`0707`） | ❌ | initramfs 常用 |
| Ar | `21 3c 61 72 63 68 3e 0a`（`!<arch>\n`） | ❌ | 静态库 |
| Cab | `4d 53 43 46`（`MSCF`） | ❌ | Windows 安装包 |
| Z（compress） | `1f 9d` | ❌ | 老式 unix compress |
| LZO | `89 4c 5a 4f 00 0d 0a 1a 0a` | ❌ | 高实时性压缩 |

> 图例：`✅` archive 模块已有完整读写；`🟡` 仅有底层压缩能力（`src/compress/`）；`❌` 未实现。

### 还差什么（按优先级）

1. **Squashfs 多压缩算法**：现仅 zlib（`SQFS_COMPRESSION_ZLIB=1`），而 `mksquashfs` 常见默认是 xz / lzo / lz4 / zstd。
2. **Xz / Zstd / Lz4 压缩流**：补齐后即可支持 `.tar.xz` / `.tar.zst` / `.tar.lz4`。
3. **RAR / ISO / Cpio 只读解包**：嵌入式与工具链场景常见需求。
4. **大文件 64 位、元数据保真、加密**：见上文"仍可继续完善的方向"。

## 六、通用"打开任意归档"命令（Archive_Command）

> **已实现**。参考 `Node_Command` / `Node_Cli_Command`（见 `src/node/`）的"命令层"范式，
> 做成 `Command` 子类 `Archive_Command`（CLI 命令 `archive`，经 `REGISTER_APP_CMD` 注册进 `app()`），
> 内置按文件魔数自动识别格式（含 squashfs）的能力，对用户"一条命令打开所有压缩包"。

### 1. 为什么放在 Command 层

- 本库的"工具"惯例就是 `Command` 子类，免费获得 `-h/--help`、选项 / 参数解析。
- 识别 + 分发属于**工具内部策略**，不污染 `Archive` 基类对外接口；
  集中在 `run_command` 内做魔数识别、工厂分发、tgz 内层探测。

### 2. 文件与命令用法

```
src/archive/Archive_Command.h   # Archive_Command(继承 Command) + 子命令类型定义
src/archive/Archive_Command.c   # run_command 分发: list/extract/create/add
```

命令聚焦归档容器本身：`create`（打包）、`extract`（解包）、`list`（列举）、`add`（追加）。
单文件的 gzip/bzip2 压缩/解压不属于归档范畴，用 `compress` 模块的能力。
命令用法（经 `xtools` 应用框架调用）：

```
./sysroot/linux/x86_64/bin/xtools archive list    <文件> [-w 通配符]      # 列出内容
./sysroot/linux/x86_64/bin/xtools archive extract <文件> [-o 输出目录]    # 解压全部 / 指定文件
./sysroot/linux/x86_64/bin/xtools archive create  <输出.sqfs|.zip|.7z|.tar|.tgz|.tbz2> <源路径>   # 按扩展名新建
./sysroot/linux/x86_64/bin/xtools archive add     <文件> <源文件>         # 向已有归档追加(目前仅 tar 支持)
```

### 3. 内部实现（识别 / 分发不对外暴露）

```c
/* 均为 Archive_Command.c 内部 static, 不写入 Archive.h */
static archive_format_e __detect_format(const char *path);         /* 读魔数识别 */
static archive_format_e __detect_format_by_name(const char *path); /* 按扩展名兜底(创建新包时) */
static Archive        *__open(allocator_t *allocator, const char *path, const char *mode); /* 识别+object_new+open */
```

格式枚举 `archive_format_e` 定义在 `Archive.h`（命令内部映射契约，见[第五节](#五常见归档--压缩格式总览)的魔数表）。
- 对外只暴露命令 `archive`，用户无需关心格式，没有 `archive_open` 这类公共函数。
- 纯识别 / 打开逻辑封装在命令内部；以后 `fshell` / 文件管理器若要"自动打开"，再把 `__detect_format` / `__open` 提升为 `Archive.h` 公共接口即可。

### 4. 识别流程

```
读前 512 字节 → 魔数匹配
  ├─ 50 4b ..  → Zip          ├─ 68 73 71 73 (hsqs) → Squashfs
  ├─ 37 7a ..  → SevenZip     ├─ 1f 8b (gzip) → 内层探测 tar → Tgz
  ├─ 42 5a 68  → bzip2 (同上) ├─ ustar @ offset 257 → Tar
  └─ 未命中    → 按扩展名兜底
```

### 5. 关键设计点与已知差异

- **读 / 写模式**：读模式先魔数识别、失败按扩展名兜底；写模式文件可能尚不存在，直接按扩展名识别（如 `out.sqfs` → Squashfs）。
- **目录路径归一**：create/extract 会把源目录 / 输出目录补结尾 `/`（`fs_get_relative_path` 与 `extracting_path` 拼接均要求），否则 tar 类格式会拼接错乱。
- **tgz / tbz2**：create 走"先打 tar 再 gzip/bzip2"；list/extract 走"先解外层流再读内层 tar"。
- **add（对已存在归档追加）**：目前仅 tar 支持；zip 打开已存在归档后保存旧 central dir 条目缺 `opaque` 回指会崩溃（Zip.c 局限），squashfs / 7z / tgz 也不支持——见能力矩阵。
- **条目名策略（已统一）**：`create` 打包目录时各格式均存"相对源目录的条目名"（zip/7z/tgz 在 `add_file` 中归一化、对齐 tar；squashfs 亦存相对名）。tar/zip/7z/tgz 一条目一条相对名（可含 `/`，子目录即路径一部分）；squashfs 写端由相对名构建真正的多级目录树（目录 inode + 嵌套目录项），子目录内文件可与其它目录同名。解压统一按相对名落盘到 `输出目录/<相对名>`（需要时建子目录），互不混淆。
- **单文件压缩流**（纯 gzip/bzip2/xz 等非归档容器）：提示"是压缩流而非归档容器"，不按归档打开。
- **扩展性**：新增格式只需三步——枚举加一项、`class_name` 加映射、魔数表加一行。

### 6. 命令能力矩阵（当前实现）

| 格式 | list | extract | create | add | 条目名 / 落盘 |
| --- | --- | --- | --- | --- | --- |
| tar | ✅ | ✅ | ✅ | ✅ | 相对名，平铺 |
| tgz / tbz2 | ✅ | ✅ | ✅ | ❌ | 内层 tar，相对名 |
| zip | ✅ | ✅ | ✅ | ❌* | 相对名，平铺；*Zip.c 不支持对已存在归档追加 |
| 7z | ✅ | ✅ | ✅ | ❌* | 相对名，平铺 |
| squashfs | ✅ | ✅ | ✅ | ❌ | 相对名→写多级目录树（支持子目录/同名） |

### 7. 魔数表（识别用）

| 格式 | 类名 | 魔数（16 进制） |
| --- | --- | --- |
| zip | `Zip` | `50 4b 03 04` / `50 4b 05 06` |
| 7z | `SevenZip` | `37 7a bc af 27 1c` |
| squashfs | `Squashfs` | `68 73 71 73` |
| tar | `Tar` | offset 257: `75 73 74 61 72` |
| tgz / tbz2 | `Tgz` / `Tbz2` | `1f 8b` / `42 5a 68` + 内层 tar 探测 |
| xz / zstd / lz4 | 待新增 | `fd 37 7a 58 5a 00` / `28 b5 2f fd` / `04 22 4d 18` |
| rar / iso / cpio / ar | 待新增 | `52 61 72 21 1a 07` / offset 0x8001 `43 44 30 30 31` / `30 37 30 37` / `21 3c 61 72 63 68 3e` |

## 七、结论

"主干完整、边缘可补"。每种格式的打包 / 解包 / 列举 + 跨工具互操作这条主线已实现得相当全面扎实，
作为嵌入式 / 通用库的 archive 能力已够用。若要对齐 `libarchive` / `7-Zip` 级别的完整度，
下一步优先级建议：

1. 通用"打开任意归档"命令 `Archive_Command` **已实现**（魔数自动识别，含 squashfs），见[第六节](#六通用打开任意归档命令archive_command)；后续可补 zip/7z 的"对已存在归档追加(add)"
2. 多压缩算法（尤其 Squashfs 的 xz / lz4，以及 xz / zstd 流）
3. 元数据保真（权限 / uid / gid / symlink）
4. 大文件 64 位支持
