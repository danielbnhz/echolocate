#pragma once
#include <cstddef>
#include <vector>

namespace echo {

constexpr float kSpeedOfSound = 343.0f; // m/s at ~20 C


std::vector<float> matchedFilter(const std::vector<float>& recording,
                                 const std::vector<float>& chirp);

struct Echo {
    size_t lagSamples;
    float  strength;
    float  distanceM;
};


std::vector<Echo> findEchoes(const std::vector<float>& mf,
                             float sampleRate,
                             float relThreshold = 0.1f,
                             size_t minSeparationSamples = 48);


std::vector<Echo> findEchoesBitwise(
    const std::vector<float>& mf,
    float sampleRate,
    float relThreshold = 0.1f,
    size_t minSeparationSamples = 48
);


}  
