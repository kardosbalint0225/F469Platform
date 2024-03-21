TODO: cwd uses functions from LuaRTOS
TODO: retarget_locks.c 

Files used from RIOT-OS Release-2024.01 08-Feb-2024
    Repository: https://github.com/RIOT-OS/RIOT
    Release:    https://github.com/RIOT-OS/RIOT/releases/tag/2024.01
    
1.)  /sys/shell/cmds/vfs.c 
2.)  /pkg/fatfs/fatfs_diskio/mtd/include/fatfs_diskio_mtd.h    
3.)  /pkg/fatfs/fatfs_diskio/mtd/mtd_diskio.c 
4.)  /pkg/fatfs/fatfs_vfs/fatfs_vfs.c
5.)  /sys/include/fs/fatfs.h 
6.)  /sys/include/iolist.h 
7.)  /drivers/include/mtd.h
8.)  /drivers/include/mtd_sdcard.h
9.)  /drivers/include/periph/rtc.h
10.) /sys/include/rtc_utils.h 
11.) /drivers/include/sdcard_spi.h -> sdcard.h 
12.) /sys/include/stdio_base.h 
13.) /sys/include/vfs.h 
14.) /sys/include/vfs_default.h 
15.) /sys/include/vfs_util.h 
16.) /sys/iolist/iolist.c 
17.) /sys/libc/include/sys/uio.h 
18.) /sys/newlib_syscalls_default/syscalls.c 
19.) /drivers/mtd/mtd.c 
20.) /drivers/mtd_sdcard/mtd_sdcard.c 
21.) /sys/posix/include/sys/statvfs.h 
22.) /sys/rtc_utils/rtc_utils.c 
23.) /drivers/sdmmc/sdmmc.c -> function prototypes only 
24.) /sys/stdio_uart/stdio_uart.c 
25.) /sys/vfs/vfs.c 
26.) /sys/vfs/vfs_stdio.c 
27.) /sys/vfs_util/vfs_util.c 

Changes made to these files:
1.)  /sys/shell/cmds/vfs.c: the whole file is changed 
2.)  /pkg/fatfs/fatfs_diskio/mtd/include/fatfs_diskio_mtd.h: no changes were made
3.)  /pkg/fatfs/fatfs_diskio/mtd/mtd_diskio.c 
    3.1.) line 28-29: preprocessor conditional changed to FF_FS_NORTC
                      #include "periph/rtc.h" changed to #include "rtc.h"
4.)  /pkg/fatfs/fatfs_vfs/fatfs_vfs.c:
    4.1.) line 32: replaced #include "mutex.h" with #include "container.h"
    4.2.) line 80-111: in function _format: removed the CONFIG_FATFS_FORMAT_ALLOC_STATIC
          sections (static buffer and mutex usage) and replaced the malloc/free calls to 
          pvPortMalloc and vPortFree FreeRTOS API calls
    4.3.) line 189: in function _statvfs: calculation of f_blocks is modified according 
          to the FatFs documentation: http://elm-chan.org/fsw/ff/doc/getfree.html
    4.4.) line 477-507: in function _fatfs_time_to_timespec: time calculation modified to
          use mktime
5.)  /sys/include/fs/fatfs.h: no changes were made                                      
6.)  /sys/include/iolist.h: no changes were made
7.)  /drivers/include/mtd.h: no changes were made
8.)  /drivers/include/mtd_sdcard.h:
    8.1.) line 10-12: defgroup and brief modified. mtd_sdcard uses the SDIO interface
    8.2.) line 27: replaced #include "sdcard_spi.h" with #include "sdcard.h"
    8.3.) line 42-43: removed sdcard and params from the struct since I did not use them elsewhere
9.)  /drivers/include/periph/rtc.h: 
    9.1.) defined _EPOCH_YEAR    
    9.2.) modified the prototype of rtc_init to int return type
    9.3.) added the prototype of rtc_deinit
    9.4.) removed the alarm and power state related function prototypes
10.) /sys/include/rtc_utils.h:  no changes were made
11.) /drivers/include/sdcard_spi.h: Based my custom sdcard driver on this header file.
12.) /sys/include/stdio_base.h 
    12.1.) Added FreeRTOS.h and queue.h header files 
    12.2.) Added stdio_deinit function prototype
    12.3.) Added stdio_add_stdin_listener function prototype
13.) /sys/include/vfs.h: 
    13.1.) line 69: Removed #include "sched.h"
    13.2.) line 400: pid is replaced by task id 
    13.3.) vfs_init and vfs_deinit function prototypes added 
14.) /sys/include/vfs_default.h:
    14.1.) line 23: Removed #include "board.h"     
15.) /sys/include/vfs_util.h: no changes were made
16.) /sys/iolist/iolist.c: no changes were made
17.) /sys/libc/include/sys/uio.h: no changes were made
18.) /sys/newlib_syscalls_default/syscalls.c: Based my custom implementation of syscalls 
     on this source file   
19.) /drivers/mtd/mtd.c: 
    19.1.) Added FreeRTOS.h, task.h, queue.h, semphr.h header files 
    19.2.) line 79: Replaced malloc by pvPortMalloc FreeRTOS API call 
20.) /drivers/mtd_sdcard/mtd_sdcard.c:
     Most of the changes are related to the interfacing of the sdcard driver 
21.) /sys/posix/include/sys/statvfs.h: no changes were made
22.) /sys/rtc_utils/rtc_utils.c: 
    22.1.) Modified #include "periph/rtc.h" to #include "rtc.h"              
    22.2.) RIOT_EPOCH replaced by _EPOCH_YEAR
23.) /drivers/sdmmc/sdmmc.c: Based my custom sdcard driver on this source file. 
24.) /sys/stdio_uart/stdio_uart.c: the whole file is changed
    24.1.) uart write is non-blocking using DMA and a gate-keeper task 
25.) /sys/vfs/vfs.c: 
    25.1.) line 29-31: Removed mutex.h, thread.h, sched.h and included
                       FreeRTOS.h, task.h, queue.h and semphr.h
    25.2.) line 145-146: Modified _open_mutex and _mount_mutex to be FreeRTOS mutexes
                         Added vfs_init and vfs_deinit functions for static mutex creation and deletion
    25.3.) Replaced mutex_lock, unlock calls to the FreeRTOS API equivalent throughout the file 
    25.4.) line 994-1014: in function _allocate_fd: pid replaced by task_id, 
                          the KERNEL_PID_UNDEF is substituted with (UBaseType_t)0U
    25.5.) line 1019-1023: See 25.4.)
    25.6.) line 1152-1156: in function vfs_sysop_stat_from_fstat:                      
26.) /sys/vfs/vfs_stdio.c: no changes were made
27.) /sys/vfs_util/vfs_util.c: no changes were made  
