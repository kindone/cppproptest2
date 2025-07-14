#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, utf8)
{
    EXPECT_FOR_ALL([](uint32_t unicode) {
        vector<uint8_t> encoded;
        util::encodeUTF8(unicode, encoded);
        uint32_t out = util::decodeUTF8(encoded);
        PROP_EXPECT_EQ(unicode, out);
    }, gen::unicode());
}

TEST(Compile, cesu8)
{
    EXPECT_FOR_ALL([](uint32_t unicode) {
        vector<uint8_t> encoded;
        util::encodeCESU8(unicode, encoded);
        uint32_t out = util::decodeCESU8(encoded);
        PROP_EXPECT_EQ(unicode, out);
    }, gen::unicode());
}

TEST(Compile, utf16be)
{
    EXPECT_FOR_ALL([](uint32_t unicode) {
        vector<uint8_t> encoded;
        util::encodeUTF16BE(unicode, encoded);
        uint32_t out = util::decodeUTF16BE(encoded);
        stringstream str;
        util::charAsHex(str, encoded);
        PROP_EXPECT_EQ(unicode, out) << util::hex << unicode << " : " << out << "(" << str.str() << ")";
    }, gen::unicode());
}

TEST(Compile, utf16le)
{
    EXPECT_FOR_ALL([](uint32_t unicode) {
        vector<uint8_t> encoded;
        util::encodeUTF16LE(unicode, encoded);
        uint32_t out = util::decodeUTF16LE(encoded);
        PROP_EXPECT_EQ(unicode, out) << util::hex << unicode << " : " << out;
    }, gen::unicode());
}

TEST(Compile, CESU8UTF16BE)
{
    vector<uint8_t> cesu8;
    cesu8.push_back(0xed);
    cesu8.push_back(0xac);
    cesu8.push_back(0xb2);
    cesu8.push_back(0xed);
    cesu8.push_back(0xb5);
    cesu8.push_back(0x91);
    uint32_t unicode = util::decodeCESU8(cesu8);

    vector<uint8_t> utf16be;
    util::encodeUTF16BE(unicode, utf16be);
    stringstream str;
    util::charAsHex(str, utf16be);
    cout << util::hex << unicode << ", " << str.str() << endl;
}

TEST(Compile, CESU8UTF16LE)
{
    //ed a7 b7 ed b4 91
    vector<uint8_t> cesu8;
    cesu8.push_back(0xed);
    cesu8.push_back(0xa6);
    cesu8.push_back(0x89);
    cesu8.push_back(0xed);
    cesu8.push_back(0xb6);
    cesu8.push_back(0xb7);

    uint32_t unicode = util::decodeCESU8(cesu8);
    util::decodeCESU8(cout, cesu8) << endl;

    vector<uint8_t> utf16le;
    util::encodeUTF16LE(unicode, utf16le);
    stringstream str;
    util::charAsHex(str, utf16le);

    cout << util::hex << "unicode: " <<unicode << ", utf16 le hex:" << str.str() << ", utf16 le decoded: " << endl;

    util::decodeUTF16LE(cout, utf16le);
    cout << endl;
}
