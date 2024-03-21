

Files used from RIOT-OS Release-2024.01 08-Feb-2024
    Repository: https://github.com/RIOT-OS/RIOT
    Release:    https://github.com/RIOT-OS/RIOT/releases/tag/2024.01

1.)  /core/lib/include/macros/units.h
2.)  /core/lib/include/macros/utils.h
3.)  /core/lib/include/assert.h
4.)  /core/lib/include/bitarithm.h
5.)  /core/lib/include/clist.h
6.)  /core/lib/include/compiler_hints.h
7.)  /core/lib/include/container.h 
8.)  /core/lib/include/debug.h 
9.)  /core/lib/include/list.h 
10.) /core/lib/include/modules.h 
11.) /core/lib/include/panic.h 
12.) /core/lib/include/xfa.h 
13.) /core/lib/assert.c 
14.) /core/lib/bitarithm.c 
15.) /core/lib/clist.c 
16.) /core/lib/panic.c 

Changes made to these files:
1.)  /core/lib/include/macros/units.h: no changes were made 
2.)  /core/lib/include/macros/utils.h: no changes were made 
3.)  /core/lib/include/assert.h:
    3.1.) line 31-64: Removed the #ifdef DOXYGEN #endif section
    3.2.) line 66-74: Removed the DOXYGEN comment on __NORETURN
    3.3.) line 85: Removed preprocessor conditional
    3.4.) line 89-90: Removed note 
    3.5.) line 102-135: Removed comments
    3.6.) line 138-141: Removed definition of _assert_panic
4.)  /core/lib/include/bitarithm.h:
    4.1.) line 26: #include "cpu_conf.h" modified to #include "core_config.h"
5.)  /core/lib/include/clist.h: 
    5.1.) line 95: #include "list.h" modified to #include "ilist.h"
6.)  /core/lib/include/compiler_hints.h: no changes were made   
7.)  /core/lib/include/container.h: no changes were made  
8.)  /core/lib/include/debug.h:
    8.1.) line 27-29: FreeRTOS.h and core_config.h included
    8.2.) line 38: modified the comment to task
    8.3.) line 47-57: modified DEBUG_PRINT to use the FreeRTOS API calls
    8.4.) line 77-78: removed #include "architecture.h" and modified the DEBUG_BREAKPOINT definition
    8.5.) line 135: DEBUG_EXTRA_STACKSIZE modified to use TASK_EXTRA_STACKSIZE 
9.)  /core/lib/include/list.h: modified the file name to ilist.h 
    9.1.) line 23-24, 102: modified the guard condition to ILIST_H
10.) /core/lib/include/modules.h: no changes were made    
11.) /core/lib/include/panic.h: the whole file is changed
    11.1.) The panic handling function name and prototype are changed 
12.) /core/lib/include/xfa.h: no changes were made      
13.) /core/lib/assert.c: the whole file is changed
    13.1.) _assert_failure implemented differently
    13.2.) _assert_panic definition removed   
14.) /core/lib/bitarithm.c: no changes were made
15.) /core/lib/clist.c: no changes were made
16.) /core/lib/panic.c: the whole file is changed 
    16.1.) my implementation only tries to initialize the PANIC_UART peripheral
           and print the error message then goes to an infinite loop     