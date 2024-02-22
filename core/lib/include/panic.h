/*
 * panic.h
 *
 *  Created on: Feb 22, 2024
 *      Author: Balint
 */

#ifndef __PANIC_H__
#define __PANIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NORETURN
#ifdef __GNUC__
#define __NORETURN __attribute__((noreturn))
#else /*__GNUC__*/
#define __NORETURN
#endif /*__GNUC__*/
#endif /*__NORETURN*/

__NORETURN void panic(const char *message, ...);

#ifdef __cplusplus
}
#endif
#endif /* __PANIC_H__ */
