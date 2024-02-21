/*
 * sdcard.h
 *
 *  Created on: 2023. okt. 25.
 *      Author: Balint
 */

#ifndef __SDCARD_H__
#define __SDCARD_H__

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
 * @retval  0           on success, @p block_num blocks were read successfully
 * @retval  -ENOTSUP    if card does not support a used command or is in wrong state
 * @retval  -EBUSY      if card is busy
 * @retval  -EFAULT     on card status error
 * @retval  -ETIMEDOUT  on timeout condition
 * @retval  -EINVAL     on invalid transfer parameters
 * @retval  -EBADMSG    on CRC7 error
 * @retval  -ENOMEM     on RX FIFO overflow error
 * @retval  -EIO        on not further specified error incl. hardware errors
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
 * @retval  0           on success, @p block_num blocks were read successfully
 * @retval  -ENOTSUP    if card does not support a used command or is in wrong state
 * @retval  -EBUSY      if card is busy
 * @retval  -EFAULT     on card status error
 * @retval  -ETIMEDOUT  on timeout condition
 * @retval  -EINVAL     on invalid transfer parameters
 * @retval  -EBADMSG    on CRC7 error
 * @retval  -ENOMEM     on TX FIFO underrun error
 * @retval  -EIO        on not further specified error incl. hardware errors
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
 * @retval  0           on success, @p block_num blocks were erased successfully
 * @retval  -ENOTSUP    if card does not support erase operation or a command used
 * @retval  -EBUSY      if card is busy
 * @retval  -EFAULT     on card status error
 * @retval  -ETIMEDOUT  on timeout condition
 * @retval  -EBADMSG    on CRC7 error in response
 * @retval  -EINVAL     on invalid erase parameters
 * @retval  -EIO        on not further specified error incl. hardware errors
 */
int sdcard_erase_blocks(uint32_t block_addr, uint16_t block_num);

/**
 * @brief   Get Capacity of SD Card
 *
 * @return  the capacity in in byte or 0 on error
 */
uint64_t sdcard_get_capacity(void);

#endif /* __SDCARD_H__ */


