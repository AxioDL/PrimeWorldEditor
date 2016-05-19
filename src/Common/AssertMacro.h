#ifndef ASSERT_H
#define ASSERT_H

#include "Log.h"
#include "TString.h"
#include <cstdlib>
#include <string.h>

/* This header declares a macro, ASSERT(Expression). ASSERT evaluates the input expression and verifies that
 * it is true. If the expression is false, an error message will be printed to the log with info on what went
 * wrong and (in debug builds) trigger a debug break. Application execution is not aborted, but if an assert
 * fails then there is usually a decent chance of the application crashing regardless.
 *
 * Note that in public release builds, asserts are compiled out entirely, so neither log messages nor debug breaks
 * will occur.
 *
 * There are two other macros defined which can be useful for debugging, but shouldn't be used as permanent error
 * checks: BREAK_ONLY_ASSERT, which doesn't write the error to the log, and LOG_ONLY_ASSERT, which doesn't trigger
 * a debug break.
 */
#define __FILE_SHORT__ strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__

#if _DEBUG
    #define DEBUG_BREAK __debugbreak();
#else
    #define DEBUG_BREAK {}
#endif

#if !PUBLIC_RELEASE

    // Development Build
    #define ASSERT_CHECK_BEGIN(Expression) \
        { \
            if (!(Expression)) \
            {

    #define ASSERT_CHECK_END \
            } \
        }

    #define WRITE_FAILURE_TO_LOG(Expression) \
        Log::Write(TString(__FILE_SHORT__) + "(" + TString::FromInt32(__LINE__, 0, 10) + "): ASSERT FAILED: " + #Expression);

    #define BREAK_ONLY_ASSERT(Expression) \
        ASSERT_CHECK_BEGIN(Expression) \
        DEBUG_BREAK \
        ASSERT_CHECK_END

    #define LOG_ONLY_ASSERT(Expression) \
        ASSERT_CHECK_BEGIN(Expression) \
        WRITE_FAILURE_TO_LOG(Expression) \
        ASSERT_CHECK_END

    #define ASSERT(Expression) \
        ASSERT_CHECK_BEGIN(Expression) \
        WRITE_FAILURE_TO_LOG(Expression) \
        DEBUG_BREAK \
        ASSERT_CHECK_END

#else

    // Public Release Build
    #define BREAK_ONLY_ASSERT(Expression) {}
    #define LOG_ONLY_ASSERT(Expression) {}
    #define ASSERT(Expression) {}

#endif

#endif // ASSERT_H
