/*
 * sdcard.h
 *
 *  Created on: 2023. okt. 25.
 *      Author: Balint
 */

#ifndef _SDMMC_H_
#define _SDCARD_H_

#include <stdint.h>

#define SDCARD_SDHC_BLOCK_SIZE   (512ul)

int sdcard_init(void);
int sdcard_deinit(void);

uint64_t sdcard_get_capacity(void);

int sdcard_read_blocks(uint32_t block_addr, uint16_t block_size,
                      uint16_t block_num, void *data, uint16_t *done);

int sdcard_write_blocks(uint32_t block_addr, uint16_t block_size,
                       uint16_t block_num, const void *data, uint16_t *done);

int sdcard_erase_blocks(uint32_t block_addr, uint16_t block_num);

#endif /* _SDCARD_H_ */


