/**
 * @ingroup     core_util
 * @{
 *
 * @file
 * @brief       Crash handling header
 *
 * Define a function that allows to stop the system
 * when an unrecoverable problem has occurred.
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

/**
 * @brief Handle an unrecoverable error by halting the system
 *
 * Detailing the failure is possible using the *message* parameter.
 *
 * @warning this function NEVER returns!
 *
 * @param[in] message       a human readable reason for the crash
 *
 * @return                  this function never returns
 * */
__NORETURN void panic(const char *message, ...);

#ifdef __cplusplus
}
#endif
#endif /* __PANIC_H__ */
/** @} */

