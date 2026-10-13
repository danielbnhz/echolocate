// Synthetic test: direct path at sample 40, one echo from a wall 1.50 m away.
// No hardware involved, so a failure here is a code bug, not a physics/hardware issue.
#include "chirp.h"
#include "ranging.h"
#include <cmath>
#include <cstdio>

int main() {
    const float fs = 48000.0f;
    auto chirp = echo::makeChirp(2000.0f, 20000.0f, 0.004f, fs);

    const float wallM = 1.50f;
    const size_t directLag = 40;
    const size_t echoLag = directLag +
        static_cast<size_t>(std::lround(2.0f * wallM / echo::kSpeedOfSound * fs));

    std::vector<float> rec(4096, 0.0f);
    for (size_t i = 0; i < chirp.size(); ++i) {
        rec[directLag + i] += 1.0f * chirp[i];   // direct speaker -> mic
        rec[echoLag + i]   += 0.3f * chirp[i];   // weaker wall echo
    }

    auto mf = echo::matchedFilter(rec, chirp);
    auto echoes = echo::findEchoes(mf, fs);

    for (auto& e : echoes)
        std::printf("lag=%zu strength=%.2f distance=%.3f m\n", e.lagSamples, e.strength, e.distanceM);

    bool ok = echoes.size() == 2 && std::fabs(echoes[1].distanceM - wallM) < 0.05f;
    std::printf(ok ? "PASS\n" : "FAIL\n");
    return ok ? 0 : 1;
}
