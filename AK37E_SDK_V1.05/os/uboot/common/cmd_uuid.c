#include <common.h>
#include <command.h>
#include <s_record.h>
#include <net.h>
#include <ata.h>
#include <part.h>
#include <fat.h>
#include <fs.h>
#include "spi.h"
#include <asm/io.h>

#define SPI_BUS_DEFAULT 0
#define SPI_CS_DEFAULT 0
#define SPI_MAX_HZ_DEFAULT 1000000
#define SPI_MODE_DEFAULT SPI_MODE_3

static int flash_jedec_id_check(struct spi_slave *slave)
{
    int ret = 0;
    unsigned char id[3] = {0};
    unsigned char cmd = 0x9F;

    ret = spi_xfer(slave, 8, &cmd, NULL, SPI_XFER_BEGIN);
    if (ret)
    {
        printf("spi cmd %#x send failed\n", cmd);
        return ret;
    }

    ret = spi_xfer(slave, 8 * sizeof(id), NULL, id, SPI_XFER_END);
    if (ret)
    {
        printf("spi read jedec id failed\n");
        return ret;
    }
    printf("flash id:0x%02x%02x%02x\n", id[2], id[1], id[0]);

    if ((id[2] != 0x18) || (id[1] != 0x40) || (id[0] != 0x20))
    {
        return -1;
    }
    return 0;
}

static int flash_unique_id_read(struct spi_slave *slave, unsigned char *id, int size)
{
    int ret = 0, i = 0;
    unsigned char cmd[5] = {0x4B, 0, 0, 0, 0, 0};

    ret = spi_xfer(slave, 8 * sizeof(cmd), &cmd, NULL, SPI_XFER_BEGIN);
    if (ret)
    {
        printf("spi cmd %#x send failed\n", cmd);
        return ret;
    }

    ret = spi_xfer(slave, 8 * size, NULL, id, SPI_XFER_END);
    if (ret)
    {
        printf("spi read unique id failed\n");
        return ret;
    }
#if 0
    printf("flash unique id number:0x");
    for (i = 0; i < size; i++)
    {
        printf("%02x", id[i]);
    }
    printf("\n");
#endif
    return ret;
}

static int security_register_read(struct spi_slave *slave, unsigned char *reg, int size)
{
    int ret = 0, i = 0;
    unsigned char addres[] = {0X48, 0x00, 0x10, 0x00, 0X00};
    unsigned char dummy_byte = 0x00;
    ret = spi_xfer(slave, 8 * sizeof(addres), &addres, NULL, SPI_XFER_BEGIN);
    if (ret)
    {
        printf("spi cmd %#x send failed\n", 0X48);
        return ret;
    }

    ret = spi_xfer(slave, 8 * size, NULL, reg, SPI_XFER_BEGIN);
    if (ret)
    {
        printf("spi read jedec id failed\n");
        return ret;
    }

    // printf("flash md5 read:0x");
    // for (i = 0; i < size; i++)
    // {
    //     printf("%02x", reg[i]);
    // }
    // printf("\n");
    return 0;
}

static void calculate_md5_value(const unsigned char *unique, int unique_size, unsigned char md5_data[16])
{
    int i = 0;
    const unsigned char key[] = "+8613342541441";
    // key +8613342541441（11byte）+ unique_size
    unsigned char string[64] = {0};

    if (unique_size + sizeof(key) > sizeof(string))
    {
        printf("There are significant issues with the code design.\n");
        while (1)
            ;
    }

    for (i = 0; i < unique_size; i++)
    {
        string[i] = unique[i];
    }
    for (i = 0; i < sizeof(key); i++)
    {
        string[unique_size + i] = key[i];
    }
#if 0
    printf("string:\"");
    for (i = 0; i < unique_size; i++)
    {
        printf("%x ", string[i]);
    }
    for (i = 0; i < sizeof(key); i++)
    {
        printf("%c ", string[unique_size + i]);
    }
    printf("\"\n");
#endif
    md5(string, unique_size + sizeof(key) - 1, md5_data);

    // printf("calculate md5:0x");
    // for (i = 0; i < 16; i++)
    // {
    //     printf("%x", md5_data[i]);
    // }
    // printf("\n");
}

static int do_uuid_check(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int ret = 0, i = 0;
    struct spi_slave *slave = NULL;
    unsigned char md5_data[16] = {0};
    unsigned char security_register_data[16] = {0};
    unsigned char unique[8] = {0};

    // printf("\n<<uuid check start>>\n");
    slave = spi_setup_slave(SPI_BUS_DEFAULT, SPI_CS_DEFAULT, SPI_MAX_HZ_DEFAULT, SPI_MODE_DEFAULT);
    if (!slave)
    {
        printf("Failed to initialize SPI flash at %u:%u\n", 0, 0);
        ret = -1;
        goto finish;
    }

    // 判断厂商id合法
    if (flash_jedec_id_check(slave))
    {
        printf("jedec id check failed\n");
        ret = -1;
        goto finish;
    }

    // 读取uuid
    if (flash_unique_id_read(slave, unique, sizeof(unique)))
    {
        printf("read unique id failed\n");
        ret = -1;
        goto finish;
    }

    // 读取保护区域的md5值
    security_register_read(slave, security_register_data, sizeof(security_register_data));

    // 计算md5
    calculate_md5_value(unique, sizeof(unique), md5_data);

    // 比对md5值是否相等
    for (i = 0; i < sizeof(md5_data); ++i)
    {
        if (md5_data[i] != security_register_data[i])
        {
            ret = -1;
            break;
        }
    }

finish:
    if (slave)
    {
        spi_free_slave(slave);
    }

    if (ret)
    {
        while (1)
        {
            printf("uuid check fail\n");
            udelay(1000 * 1000);
        }
    }
    else
    {
        printf("The chip and firmware are legitimate and will proceed to the next step of operation.\n");
    }

    return ret;
}

U_BOOT_CMD(
    uuid_check, 7, 0, do_uuid_check,
    "upgrade file from a dos filesystem",
    "<interface> [<dev[:part]>]  <addr> <filename> [bytes [pos]]\n"
    "    - Load binary file 'filename' from 'dev' on 'interface'\n"
    "      to address 'addr' from dos filesystem.\n"
    "      'pos' gives the file position to start loading from.\n"
    "      If 'pos' is omitted, 0 is used. 'pos' requires 'bytes'.\n"
    "      'bytes' gives the size to load. If 'bytes' is 0 or omitted,\n"
    "      the load stops on end of file.\n"
    "      If either 'pos' or 'bytes' are not aligned to\n"
    "      ARCH_DMA_MINALIGN then a misaligned buffer warning will\n"
    "      be printed and performance will suffer for the load.");