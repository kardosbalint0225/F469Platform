/*
 * sdmmc.h
 *
 *  Created on: 2023. okt. 25.
 *      Author: Balint
 */

#ifndef _SDMMC_H_
#define _SDMMC_H_

#include <stdint.h>

#define SDMMC_SDHC_BLOCK_SIZE   (512)

int sdmmc_init(void);
int sdmmc_deinit(void);
int sdmmc_card_init(void);
uint64_t sdmmc_get_capacity(void);

#endif /* _SDMMC_H_ */
