/*
 * hal_errno.h
 *
 *  Created on: 2023. nov. 25.
 *      Author: Balint
 */

#ifndef INCLUDE_HAL_ERRNO_H_
#define INCLUDE_HAL_ERRNO_H_

#include "stm32f4xx_hal.h"

int hal_statustypedef_to_errno(const HAL_StatusTypeDef status);

#endif /* INCLUDE_HAL_ERRNO_H_ */
