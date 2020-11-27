/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#ifndef __CCPLATFORMDEFINE_H__
#define __CCPLATFORMDEFINE_H__

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#include <assert.h>

#define CC_DLL

#define CC_ASSERT(cond) assert(cond)

////////////////////////////////////////////////////////////
// C++17 Attribute Features
//
// https://infektor.net/posts/2017-01-19-using-cpp17-attributes-today.html
//
//

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(name) 0
#endif

#define CC_UNUSED_PARAM(unusedparam) (void)unusedparam;

////////////////////////////////////////////////////////////

// c++11/14/17 attributes
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(maybe_unused)
#    define MAYBE_UNUSED [[maybe_unused]]
#  elif __has_cpp_attribute(gnu::unused)
#    define MAYBE_UNUSED [[gnu::unused]]
#  else
#    define MAYBE_UNUSED
#  endif
#else
#  define MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(nodiscard)
#    define NODISCARD [[nodiscard]]
#  elif __has_cpp_attribute(gnu::warn_unused_result)
#    define NODISCARD [[gnu::warn_unused_result]]
#  else
#    define NODISCARD
#  endif
#else
#    define NODISCARD
#endif

#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(fallthrough)
#    define FALLTHROUGH [[fallthrough]]
#  elif __has_cpp_attribute(clang::fallthrough)
#    define FALLTHROUGH [[clang::fallthrough]]
#  else
#    define FALLTHROUGH
#  endif
#else
#  define FALLTHROUGH
#endif

////////////////////////////////////////////////////////////
// Printf-like VA_ARGS for all compilers
// Passing variable name as string to function with default parameters
// https://stackoverflow.com/questions/9969490/passing-variable-name-as-string-to-function-with-default-parameters
//

#define ST_STRINGIFY_IMPL(s) #s
#define ST_STRINGIFY(s) ST_STRINGIFY_IMPL(s)
#define ST_ARG1_IMPL(a, ...) a
#define ST_ARG1(...) ST_ARG1_IMPL(__VA_ARGS__, 0)

#define MACRO_MyFuncNeedingZeroOrMoreVarArgs(...) MyFuncNeedingZeroOrMoreVarArgs(ST_STRINGIFY(ARG1(__VA_ARGS__)), __VA_ARGS__)

////////////////////////////////////////////////////////////

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#endif // CC_PLATFORM_IOS

#endif /* __CCPLATFORMDEFINE_H__*/
