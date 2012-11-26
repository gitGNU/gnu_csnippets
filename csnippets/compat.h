/*
 * Copyright (c) 2012 Allan Ference  <f.fallen45@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _COMPAT_H
#define _COMPAT_H

/**
 * Compatibility with other platforms should go here.
 */
#ifdef _WIN32
#ifndef USE_SELECT
#define USE_SELECT
#endif
#ifdef USE_EPOLL
#undef USE_EPOLL
#endif
#ifdef USE_KQUEUE
#undef USE_KQUEUE
#endif
#elif defined __linux
#if !defined USE_EPOLL && !defined USE_SELECT
#define USE_EPOLL
#endif
#else
#ifdef USE_KQUEUE
#error "kqueue isn't implemented yet."
#endif
#endif

#endif   /* _COMPAT_H */

