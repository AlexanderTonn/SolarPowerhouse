#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

inline void require(bool condition, const char* expression, const char* file, int line) {
    if (!condition) {
        std::cerr << "FAIL: " << file << ':' << line << ": " << expression << '\n';
        std::exit(EXIT_FAILURE);
    }
}

#define REQUIRE(condition) require((condition), #condition, __FILE__, __LINE__)
