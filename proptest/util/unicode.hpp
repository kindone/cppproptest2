#pragma once

#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/vector.hpp"

namespace proptest {
namespace util {

PROPTEST_API ostream& codepage(ostream& os, uint32_t code);
PROPTEST_API ostream& charAsHex(ostream& os, uint8_t c);
PROPTEST_API ostream& charAsHex(ostream& os, uint8_t c1, uint8_t c2);
PROPTEST_API ostream& charAsHex(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
PROPTEST_API ostream& charAsHex(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
PROPTEST_API ostream& charAsHex(ostream& os, vector<uint8_t>& chars);
PROPTEST_API ostream& charAsHex(ostream& os, const string& str);
PROPTEST_API ostream& validString(ostream& os, const string& str);

struct PROPTEST_API StringAsHex
{
    StringAsHex(const string& _str) : str(_str) {}

    friend ostream& operator<<(ostream& os, const StringAsHex& obj) { return charAsHex(os, obj.str); }

    string str;
};

struct PROPTEST_API StringPrintable
{
    StringPrintable(const string& _str) : str(_str) {}

    friend ostream& operator<<(ostream& os, const StringPrintable& obj) { return validString(os, obj.str); }

    string str;
};

PROPTEST_API ostream& validChar(ostream& os, uint8_t c);
PROPTEST_API ostream& validChar2(ostream& os, uint8_t c);
// ostream& validChar(ostream& os, uint8_t c1, uint8_t c2);
// ostream& validChar(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3);
// ostream& validChar(ostream& os, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);

}  // namespace util
}  // namespace proptest
