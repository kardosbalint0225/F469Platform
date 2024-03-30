/*
 * Original work Copyright (C) 2017 HAW-Hamburg
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     mtd_sdcard.c:       https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/drivers/mtd_sdcard/mtd_sdcard.c
 *
 * The original author of mtd_sdcard.c is:
 *     Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 */

/**
 * @ingroup     drivers_mtd_sdcard
 * @{
 *
 * @file        mtd_sdcard.c
 * @brief       Driver for using sdcard via mtd interface
 * @}
 */
#define ENABLE_DEBUG 0
#include "debug.h"
#include "modules.h"
#include "macros/utils.h"
#include "mtd.h"
#include "mtd_sdcard.h"
#include "sdcard.h"
#include "sdcard_config.h"

#include <inttypes.h>
#include <errno.h>
#include <string.h>

static int mtd_sdcard_init(mtd_dev_t *dev)
{
    DEBUG("mtd_sdcard_init\n");

    if (sdcard_init() == 0) {
    	
        /* erasing whole sectors is handled internally by the card so you can
           delete single blocks (i.e. pages) */
        dev->pages_per_sector = 1;
        dev->sector_count     = (uint32_t)(sdcard_get_capacity() / SDCARD_SDHC_BLOCK_SIZE);

        /* sdcard always uses the fixed block size of SD-HC cards */
        dev->page_size        = SDCARD_SDHC_BLOCK_SIZE;
        dev->write_size       = SDCARD_SDHC_BLOCK_SIZE;
        return 0;
    }
    return -EIO;
}

static int mtd_sdcard_read_page(mtd_dev_t *dev, void *buff, uint32_t page,
                                uint32_t offset, uint32_t size)
{
    int err;


    DEBUG("mtd_sdcard_read_page: page:%" PRIu32 " offset:%" PRIu32 " size:%" PRIu32 "\n",
          page, offset, size);

    if (offset || size % SDCARD_SDHC_BLOCK_SIZE) {
#if IS_USED(MODULE_MTD_WRITE_PAGE)
        if (dev->work_area == NULL) {
            DEBUG("mtd_sdcard_read_page: no work area\n");
            return -ENOTSUP;
        }

        err = sdcard_read_blocks(page, 
                                 1, 
                                 dev->work_area);
        if (err != 0) {
            return -EIO;
        }

        size = MIN(size, SDCARD_SDHC_BLOCK_SIZE - offset);
        DEBUG("mtd_sdcard_read_page: read %" PRIu32 " bytes at offset %" PRIu32 "\n",
              size, offset);
        memcpy(buff, (uint8_t *)dev->work_area + offset, size);
        return size;
#else
        return -ENOTSUP;
#endif
    }

    err = sdcard_read_blocks(page, 
                             size / SDCARD_SDHC_BLOCK_SIZE, 
                             buff);
    if (err != 0) {
        return -EIO;
    }
    return size;
}

static int mtd_sdcard_write_page(mtd_dev_t *dev, const void *buff, uint32_t page,
                                 uint32_t offset, uint32_t size)
{
    int err;


    DEBUG("mtd_sdcard_write_page: page:%" PRIu32 " offset:%" PRIu32 " size:%" PRIu32 "\n",
          page, offset, size);

    if (offset || size % SDCARD_SDHC_BLOCK_SIZE) {
#if IS_USED(MODULE_MTD_WRITE_PAGE)
        if (dev->work_area == NULL) {
            DEBUG("mtd_sdcard_write_page: no work area\n");
            return -ENOTSUP;
        }

        err = sdcard_read_blocks(page, 
                                 1, 
                                 dev->work_area);
        if (err != 0) {
            return -EIO;
        }

        size = MIN(size, SDCARD_SDHC_BLOCK_SIZE - offset);
        DEBUG("mtd_sdcard_write_page: write %" PRIu32 " bytes at offset %" PRIu32 "\n",
              size, offset);
        memcpy((uint8_t *)dev->work_area + offset, buff, size);
        err = sdcard_write_blocks(page, 
                                  1, 
                                  dev->work_area);
#else
        return -ENOTSUP;
#endif
    } else {
        err = sdcard_write_blocks(page, 
                                  size / SDCARD_SDHC_BLOCK_SIZE, 
                                  buff);
    }

    if (err != 0) {
        DEBUG("mtd_sdcard_write_page: error %d\n", err);
        return -EIO;
    }
    return size;
}

static int mtd_sdcard_erase_sector(mtd_dev_t *dev, uint32_t sector, uint32_t count)
{
#if IS_ACTIVE(CONFIG_MTD_SDCARD_ERASE) && IS_USED(MODULE_MTD_WRITE_PAGE)


    DEBUG("mtd_sdcard_erase_sector: sector: %" PRIu32 " count: %" PRIu32 "\n",
          sector, count);

    if (dev->work_area == NULL) {
        DEBUG("mtd_sdcard_erase_sector: no work area\n");
        return -ENOTSUP;
    }
    memset(dev->work_area, 0, SDCARD_SDHC_BLOCK_SIZE);
    while (count) {
        int err;
        err = sdcard_write_blocks(sector, 
                                  1, 
                                  dev->work_area);
        if (err != 0) {
            DEBUG("mtd_sdcard_erase_sector: error %d\n", err);
            return -EIO;
        }
        --count;
        ++sector;
    }
#else
    (void)dev;
    (void)sector;
    (void)count;
    if (IS_ACTIVE(CONFIG_MTD_SDCARD_ERASE)) {
        return -ENOTSUP;
    }
#endif
    return 0;
}

static int mtd_sdcard_power(mtd_dev_t *dev, enum mtd_power_state power)
{
    (void)dev;
    (void)power;

    /* TODO: implement power down of sdcard in sdcard_spi
    (make use of sdcard_spi_params_t.power pin) */
    return -ENOTSUP; /* currently not supported */
}

static int mtd_sdcard_read(mtd_dev_t *dev, void *buff, uint32_t addr,
                           uint32_t size)
{
    int res = mtd_sdcard_read_page(dev, buff, addr / SDCARD_SDHC_BLOCK_SIZE,
                                   addr % SDCARD_SDHC_BLOCK_SIZE, size);
    if (res < 0) {
        return res;
    }
    if (res == (int)size) {
        return 0;
    }
    return -EOVERFLOW;
}

const mtd_desc_t mtd_sdcard_driver = {
    .init = mtd_sdcard_init,
    .read = mtd_sdcard_read,
    .read_page = mtd_sdcard_read_page,
    .write_page = mtd_sdcard_write_page,
    .erase_sector = mtd_sdcard_erase_sector,
    .power = mtd_sdcard_power,
};

#if IS_USED(MODULE_MTD_SDCARD_DEFAULT)
#include "sdcard_config.h"
#include "vfs_default.h"



#ifndef CONFIG_SDCARD_GENERIC_MTD_OFFSET
#define CONFIG_SDCARD_GENERIC_MTD_OFFSET 0
#endif

#define MTD_SDCARD_DEV(n, m)                \
    mtd_sdcard_t mtd_sdcard_dev ## n = {    \
        .base = {                           \
           .driver = &mtd_sdcard_driver,    \
        },                                  \
                                            \
                                            \
    };                                      \
                                            \
    XFA_CONST(mtd_dev_xfa, m) mtd_dev_t CONCAT(*mtd, m) = (mtd_dev_t *)&mtd_sdcard_dev ## n

#define MTD_SDCARD_DEV_FS(n, m, filesystem) \
    VFS_AUTO_MOUNT(filesystem, VFS_MTD(mtd_sdcard_dev ## n), VFS_DEFAULT_SD(n), m)




MTD_SDCARD_DEV(0, CONFIG_SDCARD_GENERIC_MTD_OFFSET);
#ifdef MODULE_FATFS_VFS
MTD_SDCARD_DEV_FS(0, CONFIG_SDCARD_GENERIC_MTD_OFFSET, fatfs);
#endif

#endif
