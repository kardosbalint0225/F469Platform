/*
 * sdmmc.h
 *
 *  Created on: 2023. okt. 25.
 *      Author: Balint
 */

#ifndef _SDMMC_H_
#define _SDMMC_H_

#define SDMMC_SDHC_BLOCK_SIZE   (512)

int sdmmc_init(void);
int sdmmc_deinit(void);

#endif /* _SDMMC_H_ */
