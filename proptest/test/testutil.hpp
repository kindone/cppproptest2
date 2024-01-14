#pragma once

#include "proptest/Shrinkable.hpp"
#include "proptest/AnyShrinkable.hpp"
#include "proptest/util/printing.hpp"
#include "proptest/std/io.hpp"

template <typename T>
void printShrinkable(const proptest::Shrinkable<T>& shrinkable, int level) {
    for (int i = 0; i < level; i++)
        proptest::cout << "  ";

    proptest::cout << "shrinkable: " << proptest::Show<T>(shrinkable.get()) << proptest::endl;
}

template <typename T>
void printExhaustive(const proptest::Shrinkable<T>& shrinkable, int level = 0)
{
    printShrinkable(shrinkable, level);

    auto shrinks = shrinkable.getShrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        printExhaustive(shrinkable2, level + 1);
    }
}

template <typename T>
void printExhaustive(const proptest::Shrinkable<T>& shrinkable, int level, proptest::function<void(const proptest::Shrinkable<T>&, int)> func)
{
    func(shrinkable, level);

    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.template iterator<proptest::Shrinkable<T>>(); itr.hasNext();) {
        auto shrinkable2 = itr.next();
        printExhaustive(shrinkable2, level + 1, func);
    }
}

template <typename T>
bool compareShrinkable(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs, size_t maxElements = 1000)
{
    if(lhs.get() != rhs.get())
        return false;

    maxElements --;

    auto lhsShrinks = lhs.getShrinks();
    auto rhsShrinks = rhs.getShrinks();

    for(auto litr = lhsShrinks.iterator(), ritr = lhsShrinks.iterator() ; litr.hasNext() || ritr.hasNext();)
    {
        if(litr.hasNext() != ritr.hasNext())
            return false;
        if(!compareShrinkable(litr.next(), ritr.next(), maxElements))
            return false;
        maxElements --;
    }
    return true;
}

template <typename T>
void outShrinkable(proptest::ostream& stream, const proptest::Shrinkable<T>& shrinkable) {
    stream << "{value: " << proptest::Show<T>(shrinkable.get());
    auto shrinks = shrinkable.getShrinks();
    if(!shrinks.isEmpty()) {
        stream << ", shrinks: [";
        for (auto itr = shrinks.iterator(); itr.hasNext();) {
            auto shrinkable2 = itr.next();
            outShrinkable(stream, shrinkable2);
            if(itr.hasNext())
                stream << ", ";
        }
        stream << "]";
    }
    stream << "}";
}

template <typename T>
proptest::string serializeShrinkable(const proptest::Shrinkable<T>& shr)
{
    proptest::stringstream stream;
    outShrinkable(stream, shr);
    return stream.str();
}

template <typename T>
proptest::string serializeShrinkableAny(const proptest::ShrinkableAny& shr)
{
    proptest::stringstream stream;
    outShrinkable<T>(stream, shr.map<T>([](const proptest::Any& any) -> T { return any.getRef<T>(); }));
    return stream.str();
}

template <typename T>
proptest::string serializeAnyShrinkable(const proptest::AnyShrinkable& shr)
{
    proptest::stringstream stream;
    outShrinkable<T>(stream, shr.getShrinkableAny().map<T>([](const proptest::Any& any) -> T { return any.getRef<T>(); }));
    return stream.str();
}