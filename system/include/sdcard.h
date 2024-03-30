/*
 * Original work Copyright (C) 2016 Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     sdcard_spi.h:       https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/drivers/include/sdcard_spi.h
 *
 * The original author of sdcard_spi.h is:
 *     Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 */

/**
 * @defgroup    drivers_sdcard SD Card driver
 * @ingroup     drivers_storage
 * @brief       Driver for reading and writing SD Cards using the SDIO.
 * @anchor      drivers_sdcard
 * @{
 *
 * @file        sdcard.h
 * @brief       Public interface for the sdcard driver.
 *
 */

#ifndef __SDCARD_H__
#define __SDCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SDCARD_SDHC_BLOCK_SIZE   (512ul)

/**
 * @brief Initialize the SDIO peripheral and the SD card
 *
 * Initializes the GPIO, DMA and the SDIO
 *
 * @return 0 on success
 * @return < 0 on error
 */
int sdcard_init(void);

/**
 * @brief De-initialize the SDIO peripheral
 *
 * De-initializes the GPIO, DMA and the SDIO
 *
 * @return 0 on success
 * @return < 0 on error
 */
int sdcard_deinit(void);

/**
 * @brief   Read a number of blocks
 *
 * Read @p block_num blocks with size @p block_size from the sd card starting at
 * block address @p block_addr to buffer @p data.
 *
 * @pre @p data must not be `NULL`. @p block_num has to be greater than 0.
 *
 * @param[in]   block_addr  Start address to read from given as block address
 * @param[in]   block_num   Number of blocks
 * @param[out]  data        Buffer for read data
 *
 * @return  0           on success, @p block_num blocks were read successfully
 * @return  -ENOTSUP    if card does not support a used command or is in wrong state
 * @return  -EBUSY      if card is busy
 * @return  -EFAULT     on card status error
 * @return  -ETIMEDOUT  on timeout condition
 * @return  -EINVAL     on invalid transfer parameters
 * @return  -EBADMSG    on CRC7 error
 * @return  -ENOMEM     on RX FIFO overflow error
 * @return  -EIO        on not further specified error incl. hardware errors
 */
int sdcard_read_blocks(uint32_t block_addr, uint16_t block_num, void *data);

/**
 * @brief   Write a number of blocks
 *
 * Write @p block_num blocks with size @p block_size to @p dev starting at
 * block address @p block_addr from buffer @p data.
 *
 *
 * @pre @p data must not be `NULL`. @p block_num has to be greater than 0.
 *
 * @param[in]   block_addr  Start address to write to given as block address
 * @param[in]   block_num   Number of blocks
 * @param[in]   data        Buffer with data to write
 *
 * @return  0           on success, @p block_num blocks were read successfully
 * @return  -ENOTSUP    if card does not support a used command or is in wrong state
 * @return  -EBUSY      if card is busy
 * @return  -EFAULT     on card status error
 * @return  -ETIMEDOUT  on timeout condition
 * @return  -EINVAL     on invalid transfer parameters
 * @return  -EBADMSG    on CRC7 error
 * @return  -ENOMEM     on TX FIFO underrun error
 * @return  -EIO        on not further specified error incl. hardware errors
 */
int sdcard_write_blocks(uint32_t block_addr, uint16_t block_num, const void *data);

/**
 * @brief   Erase a number of blocks
 *
 * Erase @p block_num blocks starting at block address @p block_addr on
 * SD Card device.
 *
 * @pre @p block_num has to be greater than 0.
 *
 * @param[in]   block_addr  Start address for erase given as block address
 * @param[in]   block_num   Number of blocks to be erased
 *
 * @return  0           on success, @p block_num blocks were erased successfully
 * @return  -ENOTSUP    if card does not support erase operation or a command used
 * @return  -EBUSY      if card is busy
 * @return  -EFAULT     on card status error
 * @return  -ETIMEDOUT  on timeout condition
 * @return  -EBADMSG    on CRC7 error in response
 * @return  -EINVAL     on invalid erase parameters
 * @return  -EIO        on not further specified error incl. hardware errors
 */
int sdcard_erase_blocks(uint32_t block_addr, uint16_t block_num);

/**
 * @brief   Get Capacity of SD Card
 *
 * @return  the capacity in in byte or 0 on error
 */
uint64_t sdcard_get_capacity(void);

#ifdef __cplusplus
}
#endif
#endif /* __SDCARD_H__ */
/** @} */

