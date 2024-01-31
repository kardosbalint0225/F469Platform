#ifndef __HAL_ERRNO_H__
#define __HAL_ERRNO_H__

#include "stm32f4xx_hal.h"

int hal_statustypedef_to_errno(const HAL_StatusTypeDef status);

#endif /* __HAL_ERRNO_H__ */

