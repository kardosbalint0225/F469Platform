/*
 * sdmmc.h
 *
 *  Created on: 2023. okt. 25.
 *      Author: Balint
 */

#ifndef _SDMMC_H_
#define _SDMMC_H_

#include <stdint.h>

#define SDMMC_SDHC_BLOCK_SIZE   (512ul)

int sdmmc_init(void);
int sdmmc_deinit(void);

uint64_t sdmmc_get_capacity(void);

/**
 * @brief   Read a number of blocks
 *
 * Read block_num blocks with size block_size from the memory card starting at
 * block address block_addr to buffer data.
 *
 *
 * @param[in]   block_addr  Start address to read from given as block address
 * @param[in]   block_size  Block size dependent on card and mode used
 *                          - 512 bytes for block-oriented transfers (SDSC V2.x/SDHC/SDXC)
 *                          - 1, 2, 4, ... 4096 for block-oriented transfers (MMC)
 *                          - 512, 1024, 2048 for block-oriented transfers (SDSC V1.x)
 *                          - 1...512 bytes in byte/multibyte mode (SDIO only)
 *                          - 0 in stream mode (MMC only)
 * @param[in]   block_num   Number of blocks:
 *                          - 1, ... for block-oriented transfers (SD Card/MMC)
 *                          - 1 in byte/multibyte mode (SDIO only)
 *                          - 0 in stream mode (MMC only)
 * @param[out]  data        Buffer for read data
 *
 * @retval  0           on success, @p block_num blocks were read successfully
 * @retval  -ENODEV     if card is not present or not usable
 * @retval  -ENOTSUP    if card does not support a used command or is in wrong state
 * @retval  -EBUSY      if card is busy
 * @retval  -EFAULT     on card status error
 * @retval  -ETIMEDOUT  on timeout condition
 * @retval  -EINVAL     on invalid transfer parameters
 * @retval  -EBADMSG    on CRC7 error
 * @retval  -ENOMEM     on RX FIFO overflow error
 * @retval  -EIO        on not further specified error incl. hardware errors
 */
int sdmmc_read_blocks(uint32_t block_addr, uint16_t block_size,
                      uint16_t block_num, void *data, uint16_t *done);

/**
 * @brief   Write a number of blocks
 *
 * Write block_num blocks with size block_size to the memory card starting at
 * block address block_addr from buffer data.
 *
 * @param[in]   block_addr  Start address to write to given as block address
 * @param[in]   block_size  Block size dependent on card and mode used
 *                          - 512 bytes for block-oriented transfers (SDSC V2.x/SDHC/SDXC)
 *                          - 1, 2, 4, ... 4096 for block-oriented transfers (MMC)
 *                          - 512, 1024, 2048 for block-oriented transfers (SDSC V1.x)
 *                          - 1...512 bytes in byte/multibyte mode (SDIO only)
 *                          - 0 in stream mode (MMC only)
 * @param[in]   block_num   Number of blocks:
 *                          - 1, ... for block-oriented transfers (SD and MMC)
 *                          - 1 in byte/multibyte mode (SDIO only)
 *                          - 0 in stream mode (MMC only)
 * @param[in]   data        Buffer with data to write
 * @param[out]  done        Number of blocks written, can be `NULL`
 *
 * @retval  0           on success, @p block_num blocks were read successfully
 * @retval  -ENODEV     if card is not present or not usable
 * @retval  -ENOTSUP    if card does not support a used command or is in wrong state
 * @retval  -EBUSY      if card is busy
 * @retval  -EFAULT     on card status error
 * @retval  -ETIMEDOUT  on timeout condition
 * @retval  -EINVAL     on invalid transfer parameters
 * @retval  -EBADMSG    on CRC7 error
 * @retval  -ENOMEM     on TX FIFO underrun error
 * @retval  -EIO        on not further specified error incl. hardware errors
 */
int sdmmc_write_blocks(uint32_t block_addr, uint16_t block_size,
                       uint16_t block_num, const void *data, uint16_t *done);

/**
 * @brief   Erase a number of blocks
 *
 * Erase block_num blocks starting at block address block_addr on
 * SD/MMC Card device.
 *
 * @param[in]   block_addr  Start address for erase given as block address
 * @param[in]   block_num   Number of blocks to be erased
 *
 * @retval  0           on success, @p block_num blocks were erased successfully
 * @retval  -ENODEV     if card is not present or not usable
 * @retval  -ENOTSUP    if card does not support erase operation or a command used
 * @retval  -EBUSY      if card is busy
 * @retval  -EFAULT     on card status error
 * @retval  -ETIMEDOUT  on timeout condition
 * @retval  -EBADMSG    on CRC7 error in response
 * @retval  -EINVAL     on invalid erase parameters
 * @retval  -EIO        on not further specified error incl. hardware errors
 */
int sdmmc_erase_blocks(uint32_t block_addr, uint16_t block_num);

#endif /* _SDMMC_H_ */
