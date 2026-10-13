#pragma once
#include <complex>
#include <vector>

namespace echo {

using cpx = std::complex<float>;

// In-place iterative radix-2 FFT. data.size() must be a power of two.
void fft(std::vector<cpx>& data, bool inverse = false);

size_t nextPow2(size_t n);

} // namespace echo
