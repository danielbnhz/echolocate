
#pragma once

#include <cstdint>

namespace echo::bitwise {

    // Returns true if n contains exactly one set bit.
    constexpr bool isPowerOfTwo(uint64_t n) {
        return n != 0 && (n & (n - 1)) == 0;
    }

    // Sets an individual bit.
    constexpr uint64_t setBit(uint64_t value, unsigned position) {
        return position < 64
            ? value | (uint64_t{1} << position)
            : value;
    }

    // Clears an individual bit.
    constexpr uint64_t clearBit(uint64_t value, unsigned position) {
        return position < 64
            ? value & ~(uint64_t{1} << position)
            : value;
    }

    // Tests an individual bit.
    constexpr bool testBit(uint64_t value, unsigned position) {
        return position < 64 &&
            (value & (uint64_t{1} << position)) != 0;
    }

    // Clears the lowest set bit.
    constexpr uint64_t clearLowestSetBit(uint64_t value) {
        return value & (value - 1);
    }

    // Counts trailing zeros.
    // Returns 64 when the input is zero.
    constexpr unsigned countTrailingZeros(uint64_t value) {
        if (value == 0) return 64;

        unsigned count = 0;

        while ((value & 1) == 0) {
            value >>= 1;
            ++count;
        }

        return count;
    }

} // namespace echo::bitwise
