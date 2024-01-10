#include "proptest/std/exception.hpp"
#include "proptest/std/string.hpp"
#include "proptest/util/any.hpp"

using namespace proptest;

const Any Any::empty;

Any::Any(const Any& other) : ptr(other.ptr) {}

const type_info& Any::type() const {
    if(!ptr) {
        throw runtime_error("empty ptr");
    }
    return ptr->type();
}

bool Any::isEmpty() const {
    return !static_cast<bool>(ptr);
}

Any& Any::operator=(const Any& other) {
    if(!isEmpty() && !other.isEmpty() && type() != other.type())
        throw invalid_cast_error("cannot assign from " + string(type().name()) + " to " + string(other.type().name()));

    ptr = other.ptr;
    return *this;
}

// bool Any::operator==(const Any& other) {
//     if(isEmpty() || other.isEmpty())
//         return isEmpty() && other.isEmpty();

//     if(type() != other.type())
//         return false;

//     return *ptr == *other.ptr;
// }