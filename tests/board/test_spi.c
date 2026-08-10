#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libobject/mockery/mockery.h>
#include <libobject/board/hal/spi/Spi.h>

/*
 * 测试 SPI 驱动真实场景：读 SPI Flash JEDEC ID。
 *
 * 模拟 m25p80 等 SPI NOR Flash 的读 ID 操作：
 *   发送 0x9F 命令 → 读回 3 字节制造商/设备 ID。
 *   通过 write_then_read() 原子完成，CS 全程保持。
 *
 * 依赖 Linux spidev 子系统（/dev/spidevX.Y）。
 * 注意：QEMU virt 默认没有 SPI 控制器，需在 QEMU 中添加或真机运行。
 * QEMU 设备树通过 /aliases 的 spi0 把 PL022 固定到总线 0（/dev/spidev0.0），
 * 因此 SPI_TEST_BUS 为 0；真机环境可按实际总线号修改。
 */
#define SPI_TEST_BUS 0
#define SPI_TEST_CS  0

static int test_spi(TEST_ENTRY *entry)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Spi *spi = NULL;
    uint8_t cmd = 0x9F;
    uint8_t id[3] = {0};

    TRY {
        dbg_str(DBG_INFO, "test_spi");

        /* 1. 创建 Spi 对象并打开 */
        spi = object_new(allocator, "Spi", NULL);
        THROW_IF(spi == NULL, -1);

        ret = spi->open(spi, SPI_TEST_BUS, SPI_TEST_CS);
        THROW_IF(ret < 0, 1);  /* 无设备时跳过测试，返回成功 */

        /* 2. 读 JEDEC ID：发 0x9F 命令，读 3 字节 ID */
        EXEC(spi->write_then_read(spi, &cmd, 1, id, 3));
        dbg_str(DBG_INFO, "JEDEC ID: %02x %02x %02x", id[0], id[1], id[2]);

        /* 3. 关闭 */
        EXEC(spi->close(spi));

        ret = 1;
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(spi);
    }

    return ret;
}
REGISTER_TEST_CMD(test_spi);
