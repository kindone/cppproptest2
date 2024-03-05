#pragma once
#include <exception>
#include <stdexcept>
#include <system_error>
#include "proptest/std/string.hpp"

namespace proptest {

using std::error_code;
using std::exception;
using std::invalid_argument;
using std::logic_error;

class runtime_error : public std::runtime_error {
public:
    explicit runtime_error(const char* fname, int l, const char* message) : std::runtime_error(message), filename(fname), line(l) {}
    explicit runtime_error(const char* fname, int l, const std::string& message) : std::runtime_error(message.c_str()), filename(fname), line(l) {}

    const char* filename;
    int line;
};

class invalid_cast_error: public std::exception {
public:
    explicit invalid_cast_error(const char* fname, int l, const char* message) : filename(fname), line(l), msg(message) {}
    explicit invalid_cast_error(const char* fname, int l, const std::string& message) : filename(fname), line(l), msg(message) {}
    invalid_cast_error(invalid_cast_error const&) noexcept = default;

    invalid_cast_error& operator=(invalid_cast_error const&) noexcept = default;
    ~invalid_cast_error() override = default;

    const char* what() const noexcept override { return msg.c_str(); }
 private:
    const char* filename;
    int line;
    string msg;
};

} // namespace proptest