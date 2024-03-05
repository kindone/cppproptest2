#include "proptest/util/any.hpp"
#include "proptest/std/exception.hpp"
#include "proptest/std/string.hpp"

using namespace proptest;

const Any Any::empty;

Any::Any(const Any& other) : ptr(other.ptr) {}

const type_info& Any::type() const {
    if(!ptr) {
        throw runtime_error(__FILE__, __LINE__, "empty ptr");
    }
return ptr->type();
}

bool Any::isEmpty() const {
    if((ptr.get() == nullptr) != !static_cast<bool>(ptr))
        throw runtime_error(__FILE__, __LINE__, "somehow nullptr comparison misbehaves");
    return !static_cast<bool>(ptr);
    //return ptr.get() == nullptr;
}

Any& Any::operator=(const Any& other) {
    if(!isEmpty() && !other.isEmpty() && type() != other.type())
        throw invalid_cast_error(__FILE__, __LINE__, "cannot assign from " + string(type().name()) + " to " + string(other.type().name()));

    ptr = other.ptr;
    return *this;
}

Any::Any(const shared_ptr<AnyHolder>& holderPtr) : ptr(holderPtr) { }

Any Any::clone() const
{
    return Any(ptr->clone());
}

// bool Any::operator==(const Any& other) {
//     if(isEmpty() || other.isEmpty())
//         return isEmpty() && other.isEmpty();

//     if(type() != other.type())
//         return false;

//     return *ptr == *other.ptr;
// }