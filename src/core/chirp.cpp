#include "chirp.h"
#include <cmath>

namespace echo {

std::vector<float> makeChirp(float f0, float f1, float durationSec, float sampleRate) {
    const size_t n = static_cast<size_t>(durationSec * sampleRate);
    std::vector<float> out(n);
    const double pi = 3.14159265358979323846;
    const double k = (f1 - f0) / durationSec; // sweep rate, Hz/s
    for (size_t i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double phase = 2.0 * pi * (f0 * t + 0.5 * k * t * t);
        double hann = 0.5 * (1.0 - std::cos(2.0 * pi * i / (n - 1)));
        out[i] = static_cast<float>(std::sin(phase) * hann);
    }
    return out;
}

} // namespace echo
