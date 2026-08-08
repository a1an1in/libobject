#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/mtd/Mtd.h>

/*
 * 测试通用用户态 MTD 驱动（Mtd 类）的 get_info / erase / write / read /
 * write_file 接口。
 *
 * 依赖 Linux MTD 子系统（/dev/mtdN）。
 * 注意：本测试需要 QEMU 中模拟的 MTD 设备（推荐 pflash CFI NOR，详见
 * doc/board/qemu_mtd_pflash_使用指南.md），因此使用 REGISTER_TEST_CMD
 * 注册为命令（与 test_uio/test_i2c 一致），仅在具备 /dev/mtdN 的环境中运行。
 *
 * test_mtd 测试逻辑：
 *   1. 打开 /dev/mtd0，获取设备信息（size/erasesize/writesize）。
 *   2. 擦除第一个擦除块。
 *   3. 写入数据到块内偏移。
 *   4. 读回验证是否符合预期。
 *   5. 越界访问保护测试。
 *
 * test_mtd_write_file 测试逻辑：
 *   1. 创建临时文件，写入已知数据。
 *   2. 调用 write_file 刷写到 Flash。
 *   3. 读回验证。
 */

#define MTD_TEST_DEVICE   "/dev/mtd0"
#define MTD_TEST_BUF_SIZE 256

/*
 * 测试 MTD 驱动 get_info / erase / write / read 接口（单个 case）。
 */
static int test_mtd(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Mtd *mtd = NULL;
    mtd_dev_info_t info;
    uint8_t wbuf[MTD_TEST_BUF_SIZE];
    uint8_t rbuf[MTD_TEST_BUF_SIZE];
    uint32_t offset = 0;
    uint32_t i;

    TRY {
        dbg_str(DBG_INFO, "test_mtd");

        /* 1. 创建 Mtd 对象并打开 /dev/mtd0 */
        mtd = object_new(allocator, "Mtd", NULL);
        THROW_IF(mtd == NULL, -1);
        EXEC(mtd->open(mtd, MTD_TEST_DEVICE));
        dbg_str(DBG_INFO, "mtd open success, dev:%s, fd:%d",
                MTD_TEST_DEVICE, mtd->fd);

        /* 2. 获取设备信息 */
        EXEC(mtd->get_info(mtd, &info));
        dbg_str(DBG_INFO, "mtd info: size:0x%x, erasesize:0x%x, "
                "writesize:0x%x, oobsize:0x%x, type:0x%x",
                info.size, info.erasesize, info.writesize,
                info.oobsize, info.type);
        THROW_IF(info.size == 0, -1);
        THROW_IF(info.erasesize == 0, -1);

        /* 3. 擦除第一个擦除块 */
        EXEC(mtd->erase(mtd, 0, info.erasesize));
        dbg_str(DBG_INFO, "erase ok, offset:0x0, size:0x%x", info.erasesize);

        /* 4. 准备写入数据（0x00-0xFF 循环） */
        for (i = 0; i < sizeof(wbuf); i++) {
            wbuf[i] = (uint8_t)i;
        }

        /* 5. 写入数据到块内偏移（offset 0，写入 MTD_TEST_BUF_SIZE 字节） */
        EXEC(mtd->write(mtd, offset, wbuf, sizeof(wbuf)));
        dbg_str(DBG_INFO, "write ok, offset:0x%x, size:0x%x",
                offset, (uint32_t)sizeof(wbuf));

        /* 6. 读回验证是否符合预期 */
        memset(rbuf, 0, sizeof(rbuf));
        EXEC(mtd->read(mtd, offset, rbuf, sizeof(rbuf)));
        dbg_str(DBG_INFO, "read ok, offset:0x%x, size:0x%x",
                offset, (uint32_t)sizeof(rbuf));
        THROW_IF(memcmp(wbuf, rbuf, sizeof(wbuf)) != 0, -1);
        dbg_str(DBG_INFO, "write/read verify ok, data matches");

        /* 7. 越界访问保护测试：读取超出设备大小应失败 */
        ret = mtd->read(mtd, info.size, rbuf, sizeof(rbuf));
        THROW_IF(ret >= 0, -1); /* 期望失败 */

        /* 8. 关闭 */
        EXEC(mtd->close(mtd));

        /* 全部成功，返回成功标志 */
        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(mtd);
    }

    return ret;
}
REGISTER_TEST_CMD(test_mtd);

/*
 * 测试 write_file 接口：刷写大于一个擦除块的文件（验证分块读写），并读回校验。
 */
static int test_mtd_write_file(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Mtd *mtd = NULL;
    mtd_dev_info_t info;
    uint8_t *verify_buf = NULL;
    const char *test_file = "/tmp/test_mtd_write_file.bin";
    FILE *fp = NULL;
    uint32_t file_size;       /* 文件大小 > erasesize，测试多块分片 */
    uint32_t i;

    TRY {
        dbg_str(DBG_INFO, "test_mtd_write_file");

        /* 1. 创建 Mtd 对象并打开设备，获取 erasesize */
        mtd = object_new(allocator, "Mtd", NULL);
        THROW_IF(mtd == NULL, -1);
        EXEC(mtd->open(mtd, MTD_TEST_DEVICE));
        EXEC(mtd->get_info(mtd, &info));

        /* 2. 文件大小 = 2 * erasesize + 256（跨多个擦除块，验证分块读写） */
        file_size = info.erasesize * 2 + 256;
        THROW_IF(file_size > info.size, -1);

        /* 3. 创建临时文件，写入已知数据（位置相关模式：0,1,2,...） */
        fp = fopen(test_file, "wb");
        THROW_IF(fp == NULL, -1);
        for (i = 0; i < file_size; i++) {
            fputc((int)(i & 0xFF), fp);
        }
        fclose(fp);
        fp = NULL;

        dbg_str(DBG_INFO, "test file created, path:%s, size:0x%x, "
                "erasesize:0x%x, chunks:%d",
                test_file, file_size, info.erasesize,
                (file_size + info.erasesize - 1) / info.erasesize);

        /* 4. 用 write_file 刷写文件到 Flash（内部自动分块擦除+写入） */
        EXEC(mtd->write_file(mtd, 0, test_file));
        dbg_str(DBG_INFO, "write_file ok, file:%s, size:0x%x",
                test_file, file_size);

        /* 5. 校验：读回文件末尾 256 字节 */
        verify_buf = (uint8_t *)malloc(MTD_TEST_BUF_SIZE);
        THROW_IF(verify_buf == NULL, -1);
        memset(verify_buf, 0, MTD_TEST_BUF_SIZE);
        EXEC(mtd->read(mtd, file_size - MTD_TEST_BUF_SIZE,
                       verify_buf, MTD_TEST_BUF_SIZE));
        for (i = 0; i < MTD_TEST_BUF_SIZE; i++) {
            uint8_t expected = (uint8_t)((file_size - MTD_TEST_BUF_SIZE + i) & 0xFF);
            THROW_IF(verify_buf[i] != expected, -1);
        }

        /* 6. 校验：读回文件开头 256 字节 */
        memset(verify_buf, 0, MTD_TEST_BUF_SIZE);
        EXEC(mtd->read(mtd, 0, verify_buf, MTD_TEST_BUF_SIZE));
        for (i = 0; i < MTD_TEST_BUF_SIZE; i++) {
            THROW_IF(verify_buf[i] != (uint8_t)(i & 0xFF), -1);
        }
        dbg_str(DBG_INFO, "write_file verify ok (head + tail), data matches");

        /* 7. 关闭 */
        EXEC(mtd->close(mtd));

        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (fp != NULL) fclose(fp);
        if (verify_buf != NULL) free(verify_buf);
        remove(test_file);
        object_destroy(mtd);
    }

    return ret;
}
REGISTER_TEST_CMD(test_mtd_write_file);

/*
 * 测试 squashfs 文件系统挂载：write_file 刷镜像 → mount → 读文件验证。
 * squashfs 只读，兼容性好，不会像 jffs2 那样扫描 Flash 导致内核崩溃。
 * 前提：9p 已挂载到 /mnt，/mnt/test_sq.img 已由宿主机预创建。
 */
#define SQ_IMAGE_PATH   "/mnt/res/test_sq.img"
#define SQ_MOUNT_POINT  "/mnt/data"
#define SQ_TEST_FILE    "/mnt/data/test.txt"
#define SQ_EXPECT_TEXT  "hello from squashfs"

static int test_mtd_squashfs(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Mtd *mtd = NULL;
    FILE *fp = NULL;
    char buf[64];

    TRY {
        dbg_str(DBG_INFO, "test_mtd_squashfs");

        /* 1. 打开 /dev/mtd0 */
        mtd = object_new(allocator, "Mtd", NULL);
        THROW_IF(mtd == NULL, -1);
        EXEC(mtd->open(mtd, MTD_TEST_DEVICE));

        /* 2. 用 write_file 刷 squashfs 镜像到 Flash */
        EXEC(mtd->write_file(mtd, 0, SQ_IMAGE_PATH));
        dbg_str(DBG_INFO, "write_file squashfs ok, image:%s", SQ_IMAGE_PATH);
        EXEC(mtd->close(mtd));

        /* 3. 挂载 squashfs（只读） */
        system("mkdir -p " SQ_MOUNT_POINT);
        ret = system("mount -t squashfs /dev/mtdblock0 " SQ_MOUNT_POINT);
        THROW_IF(ret != 0, -1);
        dbg_str(DBG_INFO, "mount squashfs ok");

        /* 4. 读文件验证 */
        fp = fopen(SQ_TEST_FILE, "r");
        THROW_IF(fp == NULL, -1);
        memset(buf, 0, sizeof(buf));
        THROW_IF(fgets(buf, sizeof(buf), fp) == NULL, -1);
        fclose(fp);
        fp = NULL;

        buf[strcspn(buf, "\n")] = '\0';
        THROW_IF(strcmp(buf, SQ_EXPECT_TEXT) != 0, -1);
        dbg_str(DBG_INFO, "squashfs read file ok, content:%s", buf);

        /* 5. 卸载 */
        ret = system("umount " SQ_MOUNT_POINT);
        THROW_IF(ret != 0, -1);
        dbg_str(DBG_INFO, "umount squashfs ok");

        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        if (fp != NULL) fclose(fp);
        object_destroy(mtd);
    }

    return ret;
}
REGISTER_TEST_CMD(test_mtd_squashfs);
