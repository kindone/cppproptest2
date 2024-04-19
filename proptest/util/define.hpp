#pragma once

#include "proptest/util/utf8string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/utf16string.hpp"
#include "proptest/std/string.hpp"


// #define PROPTEST_ENABLE_EXPLICIT_INSTANTIATION 1

#define DEFINE_FOR_ALL_INTTYPES(DEF) \
    DEF(char);\
    DEF(int8_t);\
    DEF(int16_t);\
    DEF(int32_t);\
    DEF(int64_t);\
    DEF(uint8_t);\
    DEF(uint16_t);\
    DEF(uint32_t);\
    DEF(uint64_t);

#define DEFINE_FOR_ALL_STRINGTYPES(DEF) \
    DEF(::proptest::string);\
    DEF(::proptest::UTF8String);\
    DEF(::proptest::CESU8String);\
    DEF(::proptest::UTF16BEString);\
    DEF(::proptest::UTF16LEString);

#define DEFINE_FOR_ALL_FLOATTYPES(DEF) \
    DEF(float);\
    DEF(double);


#define DEFINE_FOR_ALL_BASIC_TYPES(DEF) \
    DEF(bool);\
    DEFINE_FOR_ALL_INTTYPES(DEF);\
    DEFINE_FOR_ALL_STRINGTYPES(DEF);\
    DEFINE_FOR_ALL_FLOATTYPES(DEF);

#define EXTERN_DECLARE_STRUCT_TYPE(TEMPLATE, TYPE) extern template struct PROPTEST_API TEMPLATE<TYPE>;
#define INSTANTIATE_STRUCT_TYPE(TEMPLATE, TYPE) template struct TEMPLATE<TYPE>;
