# archive 命令行使用说明

`archive` 是 libobject 的通用归档命令，属于 `xtools` 应用框架下的一个子命令（由
`Archive_Command` 实现，经 `REGISTER_APP_CMD` 注册）。它**按文件魔数自动识别归档格式**
（含 squashfs / zip / 7z / tar / tgz / tbz2），对用户"一条命令打开所有压缩包"，无需预先指定格式。

命令聚焦归档容器本身：`create`（打包）、`extract`（解包）、`list`（列举）、`add`（追加）。
单文件的 gzip/bzip2 压缩 / 解压不属于归档范畴，请使用 `compress` 模块的能力。

## 一、语法概览

```
./sysroot/linux/x86_64/bin/xtools archive <子命令> <arg1> [arg2] [-o 输出目录] [-w 通配符]
```

- `xtools` 可执行路径：`./sysroot/linux/x86_64/bin/xtools`（依构建产物而定）。
- 若日志刷屏，可加 `--log-type=0`（输出到终端）并自行用 `grep` 过滤。
- 在线帮助：`./sysroot/linux/x86_64/bin/xtools archive --help` 或加 `-h`。

### 参数 / 选项

| 项 | 含义 |
| --- | --- |
| `<arg0>` | 子命令：`list` / `extract` / `create` / `add` |
| `<arg1>` | 归档文件路径 |
| `<arg2>` | `create` 的源目录 / `add` 的源文件（可选） |
| `-o, --output` | `extract` 的输出目录 |
| `-w, --wildcard` | 包含通配符过滤（如 `*.txt`），用于 `list` / `extract` |
| `-h, --help` | 帮助 |

## 二、格式识别规则

1. **读**（`list`/`extract`）：读文件前 512 字节按魔数识别，未命中再按扩展名兜底。
2. **写**（`create`）：文件尚不存在，直接按输出扩展名识别（`.sqfs`→Squashfs、`.zip`→Zip、
   `.7z`→7z、`.tar`→Tar、`.tgz`→Tar+gzip、`.tbz2`→Tar+bzip2）。
3. gzip / bzip2 魔数可能是 `.tar.gz` / `.tar.bz2`，会先解外层流再探测内层 `ustar`。
4. 纯 gzip / bzip2 等**单文件压缩流（非归档容器）**：会提示"是压缩流而非归档容器"，不按归档打开。

## 三、子命令与示例

下面示例在仓库根目录执行。输入资源放在 `./tests/archive/res/`，输出统一写到 `./tests/archive/output/` 下。

### 1. list — 列出内容

```
./sysroot/linux/x86_64/bin/xtools archive list ./tests/archive/output/test.tar
# total 3 entries
# test.txt
# test2.txt
# add.txt

./sysroot/linux/x86_64/bin/xtools archive list ./tests/archive/output/test.zip -w "*.txt"   # 通配符过滤
```

### 2. extract — 解压（= 解包）

```
./sysroot/linux/x86_64/bin/xtools archive extract ./tests/archive/output/test.zip -o ./tests/archive/output/unzip
./sysroot/linux/x86_64/bin/xtools archive extract ./tests/archive/output/test.zip -o ./tests/archive/output/only -w "test.txt"
./sysroot/linux/x86_64/bin/xtools archive extract ./tests/archive/output/test.sqfs -o ./tests/archive/output/sqfs   # squashfs 也可直接解
```

> 条目名策略（已统一）：`create` 打包目录时各格式都存"相对源目录的条目名"
> （zip/7z/tgz 在 `add_file` 中归一化、对齐 tar；squashfs 也存相对名，写端再按相对名生成
> 多级目录树）。解压 `-o` 后按相对名落盘到输出目录（含子目录时会自动建子目录，如
> `sub/test.txt`），不同目录下的同名文件互不冲突。直接按默认 `adding_path` 追加单文件时，
> 条目名保持传入值。

### 3. create — 打包（= 压缩，按扩展名决定格式）

```
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/test.zip   ./tests/archive/res/cmd   # zip
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/test.7z    ./tests/archive/res/cmd   # 7z
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/test.sqfs  ./tests/archive/res/cmd   # squashfs
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/test.tar   ./tests/archive/res/cmd   # tar
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/test.tgz   ./tests/archive/res/cmd   # 先打 tar 再 gzip
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/test.tbz2  ./tests/archive/res/cmd   # 先打 tar 再 bzip2
```

### 4. add — 向已有归档追加（目前仅 tar 支持）

```
./sysroot/linux/x86_64/bin/xtools archive create ./tests/archive/output/demo.tar ./tests/archive/res/cmd
./sysroot/linux/x86_64/bin/xtools archive add    ./tests/archive/output/demo.tar ./tests/archive/res/cmd/add.txt
```

> 注意：zip / 7z / squashfs / tgz **不支持**"打开已存在归档后追加"（Zip 的追加会因旧 central dir
> 条目缺 `opaque` 回指而崩溃，属 Zip.c 局限），请先在 create 时一次性写入全部文件。

## 四、能力矩阵（当前实现）

| 格式 | list | extract | create | add | 条目名 / 落盘 |
| --- | --- | --- | --- | --- | --- |
| tar | ✅ | ✅ | ✅ | ✅ | 相对名，平铺 |
| tgz / tbz2 | ✅ | ✅ | ✅ | ❌ | 内层 tar，相对名 |
| zip | ✅ | ✅ | ✅ | ❌* | 相对名，平铺 |
| 7z | ✅ | ✅ | ✅ | ❌* | 相对名，平铺 |
| squashfs | ✅ | ✅ | ✅ | ❌ | 相对名→写多级目录树（支持子目录/同名） |

\* Zip / 7z 的 `add` 依赖"打开已存在归档后追加"，Zip.c / SevenZip.c 当前不支持。

## 五、相关文件

- 命令实现：`src/archive/Archive_Command.c`、`src/include/libobject/archive/Archive_Command.h`
- 测试：`tests/archive/test_archive_cli.c`（直接 popen 运行 `xtools archive ...` 的端到端 CLI 测试：create→list→add→extract，校验退出码/stdout/落盘）
- 输入资源：`tests/archive/res/cmd/`；输出目录：`tests/archive/output/`
