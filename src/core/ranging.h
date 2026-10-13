#pragma once
#include <cstddef>
#include <vector>

namespace echo {

constexpr float kSpeedOfSound = 343.0f; // m/s at ~20 C

// Matched filter: cross-correlate the recording against the transmitted
// chirp (via FFT). Output[i] = |correlation| at lag i samples.
std::vector<float> matchedFilter(const std::vector<float>& recording,
                                 const std::vector<float>& chirp);

struct Echo {
    size_t lagSamples;   // lag of the peak, in samples
    float  strength;     // peak height relative to the strongest peak (0..1)
    float  distanceM;    // distance beyond the direct speaker->mic arrival
};

// Finds peaks in the matched-filter output. The first peak above threshold is
// treated as the direct speaker->mic path (time zero); later peaks are echoes.
// This removes the need to know hardware latency.
std::vector<Echo> findEchoes(const std::vector<float>& mf,
                             float sampleRate,
                             float relThreshold = 0.1f,
                             size_t minSeparationSamples = 48);

} // namespace echo
