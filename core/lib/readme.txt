

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