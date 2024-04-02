/*
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     system_cwd
 * @{
 * @file        cwd.h
 * @brief       Implementation of Current Working Directory (CWD),
 *              chdir and getcwd functions
 */
#ifndef __CWD_H__
#define __CWD_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize the CWD
 *         Creates the _cwd_mutex, clears _cwd
 *
 * @param  None
 *
 * @retval None
 */
void cwd_init(void);

/**
 * @brief  De-initialize the CWD
 *         Deletes the _cwd_mutex
 *
 * @param  None
 *
 * @retval None
 */
void cwd_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* __CWD_H__ */
/** @} */
