#ifndef ASSERT_H
#define ASSERT_H

#include "Log.h"
#include "TString.h"
#include <cstdlib>
#include <string.h>

/* This header declares a macro, ASSERT(Expression). ASSERT evaluates the input expression and verifies that
 * it is true. If the expression is false, an error message will be printed to the log with info on what went
 * wrong and (in debug builds) trigger a debug break. Application execution is aborted. In public release builds,
 * asserts are compiled out entirely, so neither log messages nor debug breaks will occur.
 *
 * Alternatively, this file also declares an ENSURE macro, which is guaranteed always executes and will never be
 * compiled out, regardless of build configuration.
 */
#define __FILE_SHORT__ strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__

#if _DEBUG
    #define DEBUG_BREAK __debugbreak();
    #define CONDITIONAL_BREAK(Condition) if (##Condition) DEBUG_BREAK
#else
    #define DEBUG_BREAK {}
    #define CONDITIONAL_BREAK(Condition) {}
#endif

#define ASSERT_CHECK_BEGIN(Expression) \
    { \
        if (!(Expression)) \
        {

#define ASSERT_CHECK_END \
        } \
    }

#define WRITE_FAILURE_TO_LOG(Expression) \
    Log::Write(TString(__FILE_SHORT__) + "(" + TString::FromInt32(__LINE__, 0, 10) + "): ASSERT FAILED: " + #Expression);

// ENSURE macro always executes, never gets compiled out
#define ENSURE(Expression) \
    ASSERT_CHECK_BEGIN(Expression) \
    WRITE_FAILURE_TO_LOG(Expression) \
    DEBUG_BREAK \
    abort(); \
    ASSERT_CHECK_END

#if !PUBLIC_RELEASE
    #define ASSERT(Expression) ENSURE(Expression)
#else
    #define ASSERT(Expression) {}
#endif

#endif // ASSERT_H
