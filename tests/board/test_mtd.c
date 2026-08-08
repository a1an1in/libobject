#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/mtd/Mtd.h>

/*
 * 测试通用用户态 MTD 驱动（Mtd 类）的 get_info / erase / write / read 接口。
 *
 * 依赖 Linux MTD 子系统（/dev/mtdN）。
 * 注意：本测试需要 QEMU 中模拟的 MTD 设备（推荐 pflash CFI NOR，详见
 * doc/board/qemu_mtd_pflash_使用指南.md），因此使用 REGISTER_TEST_CMD
 * 注册为命令（与 test_uio/test_i2c 一致），仅在具备 /dev/mtdN 的环境中运行。
 *
 * 测试逻辑：
 *   1. 打开 /dev/mtd0，获取设备信息（size/erasesize/writesize）。
 *   2. 擦除第一个擦除块。
 *   3. 写入数据到块内偏移。
 *   4. 读回验证是否符合预期。
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
