#include "hal_errno.h"
#include <errno.h>

int hal_statustypedef_to_errno(const HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK      : return 0;
        case HAL_ERROR   : return -EIO;
        case HAL_BUSY    : return -EBUSY;
        case HAL_TIMEOUT : return -EBUSY;
    }

    return (int)status;
}

