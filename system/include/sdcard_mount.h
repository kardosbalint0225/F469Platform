/*
 * sdcard_mount.h
 *
 *  Created on: 2023. nov. 13.
 *      Author: Balint
 */

#ifndef __SDCARD_MOUNT_H__
#define __SDCARD_MOUNT_H__

#ifdef __cplusplus
extern "C" {
#endif

int sdcard_mount_init(void);
int sdcard_mount_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* __SDCARD_MOUNT_H__ */
