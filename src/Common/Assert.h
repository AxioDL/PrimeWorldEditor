#ifndef ASSERT_H
#define ASSERT_H

#include "Log.h"
#include "TString.h"
#include <cstdlib>
#include <string.h>

#define __FILE_SHORT__ strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__

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
    __debugbreak(); \
    ASSERT_CHECK_END

#define LOG_ONLY_ASSERT(Expression) \
    ASSERT_CHECK_BEGIN(Expression) \
    WRITE_FAILURE_TO_LOG(Expression) \
    ASSERT_CHECK_END

#define ASSERT(Expression) \
    ASSERT_CHECK_BEGIN(Expression) \
    WRITE_FAILURE_TO_LOG(Expression) \
    __debugbreak(); \
    ASSERT_CHECK_END

#endif // ASSERT_H
