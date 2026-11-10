#pragma once
#include <complex>
#include <vector>

namespace echo {

using cpx = std::complex<float>;


void fft(std::vector<cpx>& data, bool inverse = false);

size_t nextPow2(size_t n);

}
