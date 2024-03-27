/*
 * Copyright (C) 2020 ML!PA Consulting GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     units.h:            https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/include/macros/units.h
 *
 * The original author of units.h is:
 *     Benjamin Valentin <benjamin.valentin@ml-pa.com>
 */

/**
 * @ingroup     core_macros
 * @{
 *
 * @file        units.h
 * @brief       Unit helper macros
 *
 */

#ifndef MACROS_UNITS_H
#define MACROS_UNITS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   A macro to return the bytes in x KiB
 */
#define KiB(x) ((unsigned long)(x) << 10)

/**
 * @brief   A macro to return the bytes in x MiB
 */
#define MiB(x) (KiB(x) << 10)

/**
 * @brief   A macro to return the bytes in x GiB
 */
#define GiB(x) ((unsigned long long)MiB(x) << 10)

/**
 * @brief   A macro to return the Hz in x kHz
 */
#define KHZ(x)    ((x) * 1000UL)

/**
 * @brief   A macro to return the Hz in x MHz
 */
#define MHZ(x) (KHZ(x) * 1000UL)

/**
 * @brief   A macro to return the Hz in x GHz
 */
#define GHZ(x) (MHZ(x) * 1000ULL)

#ifdef __cplusplus
}
#endif

#endif /* MACROS_UNITS_H */
/** @} */
