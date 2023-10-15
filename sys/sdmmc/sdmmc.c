/*
 * sdmmc.c
 *
 *  Created on: 2023. okt. 15.
 *      Author: Balint
 */
#include "sdmmc.h"

void sdmmc_init(sdmmc_dev_t *dev)
{

}

int sdmmc_card_init(sdmmc_dev_t *dev)
{

}

int sdmmc_read_blocks(sdmmc_dev_t *dev,
                      uint32_t block_addr,
                      uint16_t block_size,
                      uint16_t block_num,
                      void *data,
                      uint16_t *done)
{

}

int sdmmc_write_blocks(sdmmc_dev_t *dev,
                       uint32_t block_addr,
                       uint16_t block_size,
                       uint16_t block_num,
                       const void *data,
                       uint16_t *done)
{

}

int sdmmc_erase_blocks(sdmmc_dev_t *dev, uint32_t block_addr, uint16_t block_num)
{

}

uint64_t sdmmc_get_capacity(sdmmc_dev_t *dev)
{

}


