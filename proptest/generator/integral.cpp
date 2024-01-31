#include "proptest/generator/integral.hpp"


namespace proptest {

DEFINE_FOR_ALL_INTTYPES(DEFINE_INTEGERS);

Shrinkable<char> Arbi<char>::operator()(Random& rand) const
{
    return generateInteger<char>(rand);
}

Shrinkable<int8_t> Arbi<int8_t>::operator()(Random& rand) const
{
    return generateInteger<int8_t>(rand);
}

Shrinkable<int16_t> Arbi<int16_t>::operator()(Random& rand) const
{
    return generateInteger<int16_t>(rand);
}

Shrinkable<int32_t> Arbi<int32_t>::operator()(Random& rand) const
{
    return generateInteger<int32_t>(rand);
}

Shrinkable<int64_t> Arbi<int64_t>::operator()(Random& rand) const
{
    return generateInteger<int64_t>(rand);
}

Shrinkable<uint8_t> Arbi<uint8_t>::operator()(Random& rand) const
{
    return generateInteger<uint8_t>(rand);
}

Shrinkable<uint16_t> Arbi<uint16_t>::operator()(Random& rand) const
{
    return generateInteger<uint16_t>(rand);
}

Shrinkable<uint32_t> Arbi<uint32_t>::operator()(Random& rand) const
{
    return generateInteger<uint32_t>(rand);
}

Shrinkable<uint64_t> Arbi<uint64_t>::operator()(Random& rand) const
{
    return generateInteger<uint64_t>(rand);
}

// template instantiation
DEFINE_FOR_ALL_INTTYPES(DEFINE_GENERATEINTEGER);
DEFINE_FOR_ALL_INTTYPES(DEFINE_NATURAL);
DEFINE_FOR_ALL_INTTYPES(DEFINE_NONNEGATIVE);
DEFINE_FOR_ALL_INTTYPES(DEFINE_INTERVAL);
DEFINE_FOR_ALL_INTTYPES(DEFINE_INRANGE);


}  // namespace proptest
