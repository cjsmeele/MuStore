/**
 * \file
 * \brief     Miniscule unit testing framework.
 * \author    Chris Smeele
 */
#pragma once

/* Copyright (c) 2016 Chris Smeele
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Example usage:
 *
 * #include "test.hh"
 * 
 * TEST(arithmetic) {
 *     LOG("This is an interesting piece of information that will be written to some log file");
 *
 *     ASSERT(1 + 1 == 2, "one plus one is two (result = %d)",   1 + 1);
 *     ASSERT(1 - 1 == 0, "one minus one is zero (result = %d)", 1 - 1);
 * }
 * 
 * TEST_MAIN() {
 *     TEST_START();
 * 
 *     RUN_TEST(arithmetic);
 * 
 *     TEST_END();
 * }
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_NAME(name) _test_ ## name

#define TEST(name)                                              \
    void TEST_NAME(name) (int *_progress, bool *_result, char **_result_text)

#define TEST_FAIL(...)                   \
    *_result = false;                    \
    asprintf(_result_text, __VA_ARGS__); \
    return;

#define ASSERT(x, ...)          \
    if ((x)) {                  \
        (*_progress)++;         \
    } else {                    \
        TEST_FAIL(__VA_ARGS__); \
    }

#define TEST_MAIN() \
    int main(void)

#define TEST_START()                           \
    bool _result = false;                      \
    char *_result_desc = NULL;                 \
    int _progress  = 0;                        \
    int _succeeded = 0;                        \
    int _failed    = 0;                        \
    int _test_i    = 1;                        \
    _test_logfile = fopen("tests.log", "a");   \
    if (!_test_logfile)                        \
        _test_logfile = stderr;

#define TEST_END()                                                      \
    printf("\ntests succeeded: %d, failed: %d\n\n", _succeeded, _failed); \
    if (_test_logfile)                                                  \
        fclose(_test_logfile);                                          \
    return 0;

#define RUN_TEST(name)                                                  \
    fprintf(_test_logfile, "----[ %s: %s() ]----\n", __FILE__, #name);  \
    printf("%2d. %s", _test_i++, #name);                                \
    for (int i=0; i<24-(int)strlen(#name); i++)                         \
        putchar('.');                                                   \
    printf(": ");                                                       \
    _result = true;                                                     \
    _progress = 0;                                                      \
    _result_desc = NULL;                                                \
    TEST_NAME(name)(&_progress, &_result, &_result_desc);               \
    if (_result) {                                                      \
        printf("\x1b[1;32m" "pass" "\x1b[0m %d/%d\n", _progress, _progress); \
        _succeeded++;                                                   \
    } else {                                                            \
        printf("\x1b[1;31m" "fail" "\x1b[0m at %d (%s)\n", _progress, _result_desc); \
        if (_result_desc)                                               \
            free(_result_desc);                                         \
        _failed++;                                                      \
    }

#define LOG(...)                                           \
    fprintf(_test_logfile, "%s:%d: ", __FILE__, __LINE__); \
    fprintf(_test_logfile, __VA_ARGS__);                   \
    fprintf(_test_logfile, "\n");                          \

FILE *_test_logfile = NULL;
