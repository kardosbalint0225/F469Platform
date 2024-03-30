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
 *     mtd_sdcard.h:       https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/drivers/include/mtd_sdcard.h
 *
 * The original author of mtd_sdcard.h is:
 *     Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 */

/**
 * @defgroup    drivers_mtd_sdcard MTD wrapper for SD Cards
 * @ingroup     drivers_storage
 * @brief       Driver for SD Cards using the MTD interface
 *
 * @{
 *
 * @file        mtd_sdcard.h
 * @brief       Interface definition for the mtd_sdcard driver
 *
 */

#ifndef MTD_SDCARD_H
#define MTD_SDCARD_H

#include <stdint.h>

#include "sdcard.h"
#include "mtd.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Device descriptor for mtd_sdcard device
 *
 * This is an extension of the @c mtd_dev_t struct
 */
typedef struct {
    mtd_dev_t base;                    /**< inherit from mtd_dev_t object */
} mtd_sdcard_t;

/**
 * @defgroup drivers_mtd_sdcard_config     SDCard driver compile configuration
 * @ingroup config_drivers_storage
 * @{
 */
/**
 * @brief   Enable SDCard Erase
 * @note    SDCards handle sector erase internally so it's
 *          possible to directly write to the card without erasing
 *          the sector first.
 *          Attention: an erase call will therefore NOT touch the content,
 *          so enable this feature to ensure overriding the data.
 *
 *          This feature requires the `mtd_write_page` module.
 */
#ifdef DOXYGEN
#define CONFIG_MTD_SDCARD_ERASE
#endif
/** @} */

/**
 * @brief   sdcard device operations table for mtd
 */
extern const mtd_desc_t mtd_sdcard_driver;

#ifdef __cplusplus
}
#endif

#endif /* MTD_SDCARD_H */
/** @} */
