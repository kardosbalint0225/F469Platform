/*
 * sysclk.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef _SYSCLK_H_
#define _SYSCLK_H_

#include <stdint.h>

enum _SYSCLK_ERROR
{
    SYSCLK_ERROR_RCC_OSC_CONFIG       = 0x00000001UL,
    SYSCLK_ERROR_PWR_ENABLE_OVERDRIVE = 0x00000002UL,
    SYSCLK_ERROR_RCC_CLOCK_CONFIG     = 0x00000004UL,

    SYSCLK_ERROR_UINT_MAX             = 0xFFFFFFFFUL,
};

void     sysclk_init(void);
uint32_t sysclk_get_error(void);

#endif /* _SYSCLK_H_ */

