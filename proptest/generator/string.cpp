#include "proptest/generator/string.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/shrinker/string.hpp"

namespace proptest {

size_t Arbi<string>::defaultMinSize = 0;
size_t Arbi<string>::defaultMaxSize = 200;

// defaults to ascii characters

Arbi<string>::Arbi(size_t _minSize, size_t _maxSize) : ArbiContainer<string>(_minSize, _maxSize), elemGen(interval<char>(0x1, 0x7f)) {}

Arbi<string>::Arbi(Generator<char> _elemGen, size_t _minSize, size_t _maxSize) : ArbiContainer<string>(_minSize, _maxSize), elemGen(_elemGen) {}

Shrinkable<string> Arbi<string>::operator()(Random& rand) const
{
    size_t size = rand.getRandomSize(minSize, maxSize + 1);
    string str(size, ' ' /*, allocator()*/);
    for (size_t i = 0; i < size; i++)
        str[i] = elemGen(rand).get();

    return shrinkString(str, minSize);
}

}  // namespace proptest
