#include "fft.h"
#include <cmath>
#include <stdexcept>
#include <utility>

namespace echo {

size_t nextPow2(size_t n) {
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

void fft(std::vector<cpx>& a, bool inverse) {
    const size_t n = a.size();
    if (n == 0 || (n & (n - 1)) != 0)
        throw std::invalid_argument("fft size must be a power of two");

    // bit-reversal permutation
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    const float pi = 3.14159265358979323846f;
    for (size_t len = 2; len <= n; len <<= 1) {
        float ang = 2.0f * pi / static_cast<float>(len) * (inverse ? 1.0f : -1.0f);
        cpx wlen(std::cos(ang), std::sin(ang));
        for (size_t i = 0; i < n; i += len) {
            cpx w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k) {
                cpx u = a[i + k];
                cpx v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (inverse)
        for (auto& x : a) x /= static_cast<float>(n);
}

} // namespace echo
