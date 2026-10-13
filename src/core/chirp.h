#pragma once
#include <vector>

namespace echo {

// Linear sweep f0 -> f1 (Hz) lasting durationSec, Hann-windowed so the
// speaker doesn't click. Returns mono float samples in [-1, 1].
std::vector<float> makeChirp(float f0, float f1, float durationSec, float sampleRate);

} // namespace echo
