#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;


TEST(Compile, generator_bool)
{
    gen::boolean gen_bool;
}

TEST(Compile, generator_int)
{
    gen::int32 gen_int;
    Arbi<double> gen_double;
    Arbi<float> gen_float;
    Arbi<uint64_t> gen_uint64;
}

TEST(Compile, generator_string)
{
    gen::string gen_string;
    Arbi<CESU8String> gen_cesu8string;
    gen::utf8string gen_utf8string;
    gen::utf16lestring gen_utf16lestring;
    gen::utf16bestring gen_utf16bestring;
}

TEST(Compile, generator_vector)
{
    Arbi<vector<int>> gen_vector_int;
    Arbi<vector<double>> gen_vector_double;
    Arbi<vector<float>> gen_vector_float;
    Arbi<vector<uint64_t>> gen_vector_uint64;
}

TEST(Compile, generator_set)
{
    Arbi<set<int>> gen_set_int;
    Arbi<set<double>> gen_set_double;
    Arbi<set<float>> gen_set_float;
    Arbi<set<uint64_t>> gen_set_uint64;
}

TEST(Compile, generator_list)
{
    Arbi<list<int>> gen_list_int;
    Arbi<list<double>> gen_list_double;
    Arbi<list<float>> gen_list_float;
    Arbi<list<uint64_t>> gen_list_uint64;
}

TEST(Compile, generator_pair)
{
    Arbi<pair<int, int>> gen_pair_int;
    Arbi<pair<double, double>> gen_pair_double;
    Arbi<pair<float, float>> gen_pair_float;
    Arbi<pair<uint64_t, uint64_t>> gen_pair_uint64;
}

