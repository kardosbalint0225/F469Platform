/*
 * Copyright (C) 2017 HAW-Hamburg
 *               2023 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_mtd_sdmmc
 * @{
 *
 * @file
 * @brief       Driver for using sdmmc via mtd interface
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 * @author      Gunar Schorcht <gunar@schorcht.net>
 *
 * @}
 */

#include <errno.h>
#include <inttypes.h>
#include <string.h>

//#define ENABLE_DEBUG 0
#include "debug.h"
//#include "kernel_defines.h"
#include "macros/utils.h"
#include "mtd.h"
#include "mtd_sdmmc.h"
#include "sdcard.h"

static int mtd_sdmmc_init(mtd_dev_t *dev)
{
    return 0;
}

static int mtd_sdmmc_read_page(mtd_dev_t *dev, void *buff, uint32_t page,
                                uint32_t offset, uint32_t size)
{
    return size;
}

static int mtd_sdmmc_write_page(mtd_dev_t *dev, const void *buff, uint32_t page,
                                 uint32_t offset, uint32_t size)
{
    return size;
}

static int mtd_sdmmc_erase_sector(mtd_dev_t *dev, uint32_t sector, uint32_t count)
{
    return 0;
}

static int mtd_sdmmc_power(mtd_dev_t *dev, enum mtd_power_state power)
{
    (void)dev;
    (void)power;

    /* TODO: implement power down of sdcard in sdcard_spi
    (make use of sdcard_spi_params_t.power pin) */
    return -ENOTSUP; /* currently not supported */
}

static int mtd_sdmmc_read(mtd_dev_t *dev, void *buff, uint32_t addr,
                           uint32_t size)
{
    int res = mtd_sdmmc_read_page(dev, buff, addr / SDCARD_SDHC_BLOCK_SIZE,
                                   addr % SDCARD_SDHC_BLOCK_SIZE, size);
    if (res < 0) {
        return res;
    }
    if (res == (int)size) {
        return 0;
    }
    return -EOVERFLOW;
}

static int mtd_sdmmc_write(mtd_dev_t *dev, const void *buff, uint32_t addr,
                            uint32_t size)
{
    int res =  mtd_sdmmc_write_page(dev, buff, addr / SDCARD_SDHC_BLOCK_SIZE,
                                     addr % SDCARD_SDHC_BLOCK_SIZE, size);
    if (res < 0) {
        return res;
    }
    if (res == (int)size) {
        return 0;
    }
    return -EOVERFLOW;
}

const mtd_desc_t mtd_sdmmc_driver = {
    .init = mtd_sdmmc_init,
    .read = mtd_sdmmc_read,
    .read_page = mtd_sdmmc_read_page,
    .write = mtd_sdmmc_write,
    .write_page = mtd_sdmmc_write_page,
    .erase_sector = mtd_sdmmc_erase_sector,
    .power = mtd_sdmmc_power,
};

