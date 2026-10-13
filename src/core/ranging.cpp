#include "ranging.h"
#include "fft.h"
#include <algorithm>
#include <cmath>

namespace echo {

std::vector<float> matchedFilter(const std::vector<float>& rec,
                                 const std::vector<float>& chirp) {
    if (rec.empty() || chirp.empty()) return {};
    const size_t n = nextPow2(rec.size() + chirp.size() - 1);

    std::vector<cpx> R(n), C(n);
    for (size_t i = 0; i < rec.size(); ++i)   R[i] = rec[i];
    for (size_t i = 0; i < chirp.size(); ++i) C[i] = chirp[i];

    fft(R); fft(C);
    for (size_t i = 0; i < n; ++i) R[i] *= std::conj(C[i]); // cross-correlation
    fft(R, true);

    std::vector<float> out(rec.size());
    for (size_t i = 0; i < rec.size(); ++i) out[i] = std::abs(R[i]);
    return out;
}

std::vector<Echo> findEchoes(const std::vector<float>& mf, float sampleRate,
                             float relThreshold, size_t minSep) {
    std::vector<Echo> result;
    if (mf.size() < 3) return result;

    const float maxv = *std::max_element(mf.begin(), mf.end());
    if (maxv <= 0.0f) return result;
    const float thresh = maxv * relThreshold;

    std::vector<size_t> peaks;
    for (size_t i = 1; i + 1 < mf.size(); ++i) {
        if (mf[i] >= thresh && mf[i] > mf[i - 1] && mf[i] >= mf[i + 1]) {
            if (!peaks.empty() && i - peaks.back() < minSep) {
                if (mf[i] > mf[peaks.back()]) peaks.back() = i; // keep the taller
            } else {
                peaks.push_back(i);
            }
        }
    }
    if (peaks.empty()) return result;

    const size_t t0 = peaks.front(); // direct path = time zero
    for (size_t p : peaks) {
        Echo e;
        e.lagSamples = p;
        e.strength = mf[p] / maxv;
        e.distanceM = static_cast<float>(p - t0) / sampleRate * kSpeedOfSound * 0.5f; // round trip
        result.push_back(e);
    }
    return result;
}

} // namespace echo
