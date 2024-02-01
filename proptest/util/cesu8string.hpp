#pragma once
#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/std/io.hpp"
#include "proptest/std/string.hpp"
#include "proptest/std/vector.hpp"

namespace proptest {

class PROPTEST_API CESU8String : public string {
public:
    explicit CESU8String(string&& other) : string(other) {}
    explicit CESU8String(string& other) : string(other) {}
    using string::string;

    size_t charsize() const;
};

namespace util {
PROPTEST_API ostream& CESU8ToHex(ostream& os, vector<uint8_t>& chars);
PROPTEST_API ostream& decodeCESU8(ostream& os, vector<uint8_t>& chars);
PROPTEST_API ostream& decodeCESU8(ostream& os, const string& str);
PROPTEST_API ostream& decodeCESU8(ostream& os, const CESU8String& str);

PROPTEST_API uint32_t decodeCESU8(vector<uint8_t>& chars);
PROPTEST_API void encodeCESU8(uint32_t utf32, vector<uint8_t>& chars);


struct PROPTEST_API DecodeCESU8
{
    DecodeCESU8(const string& _str) : str(_str) {}
    DecodeCESU8(const CESU8String& _str) : str(_str) {}
    friend ostream& operator<<(ostream& os, const DecodeCESU8& obj) { return decodeCESU8(os, obj.str); }

    string str;
};

PROPTEST_API bool isValidCESU8(vector<uint8_t>& chars);
PROPTEST_API bool isValidCESU8(vector<uint8_t>& chars, size_t& numChars);
PROPTEST_API size_t CESU8CharSize(const string& str);
}  // namespace util

}  // namespace proptest
