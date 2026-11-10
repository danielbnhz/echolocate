

#include "../src/core/chirp.h"
#include "../src/core/ranging.h"
#include <cmath>
#include <cstdio>
#include "../src/core/bitwise/bitops.h"
#include <vector>
#include <cassert>
#include <cstdint>

void testBitwiseUtilities() {
    using namespace echo::bitwise;

    assert(isPowerOfTwo(1));
    assert(isPowerOfTwo(64));
    assert(!isPowerOfTwo(0));
    assert(!isPowerOfTwo(63));

    uint64_t flags = 0;

    flags = setBit(flags, 5);
    assert(testBit(flags, 5));

    flags = setBit(flags, 12);
    assert(testBit(flags, 12));

    flags = clearBit(flags, 5);
    assert(!testBit(flags, 5));
    assert(testBit(flags, 12));

    assert(clearLowestSetBit(0b10110000) == 0b10100000);

    assert(countTrailingZeros(0b1000) == 3);
    assert(countTrailingZeros(0b100000) == 5);
    assert(countTrailingZeros(0) == 64);
}
int main() {
    testBitwiseUtilities();
    std::printf("Bitwise utilities: PASS\n");

    const float fs = 48000.0f;
    auto chirp = echo::makeChirp(
        2000.0f, 20000.0f, 0.004f, fs
    );

    const float wallM = 1.50f;
    const size_t directLag = 40;

    const size_t echoLag = directLag +
        static_cast<size_t>(
            std::lround(2.0f * wallM / echo::kSpeedOfSound * fs)
        );

    std::vector<float> rec(4096, 0.0f);

    for (size_t i = 0; i < chirp.size(); ++i) {
        rec[directLag + i] += chirp[i];
        rec[echoLag + i] += 0.3f * chirp[i];
    }

    auto mf = echo::matchedFilter(rec, chirp);


    auto scalarEchoes = echo::findEchoes(mf, fs);
    auto bitwiseEchoes = echo::findEchoesBitwise(mf, fs);

    bool same = scalarEchoes.size() == bitwiseEchoes.size();

    if (same) {
        for (size_t i = 0; i < scalarEchoes.size(); ++i) {
            if (scalarEchoes[i].lagSamples !=
                    bitwiseEchoes[i].lagSamples ||
                std::fabs(scalarEchoes[i].strength -
                          bitwiseEchoes[i].strength) > 0.000001f ||
                std::fabs(scalarEchoes[i].distanceM -
                          bitwiseEchoes[i].distanceM) > 0.000001f) {
                same = false;
                break;
                          }
        }
    }

    std::printf("Scalar vs bitwise: %s\n",
                same ? "PASS" : "FAIL");
    auto echoes = echo::findEchoes(mf, fs);

    for (auto& e : echoes) {
        std::printf(
            "lag=%zu strength=%.2f distance=%.3f m\n",
            e.lagSamples, e.strength, e.distanceM
        );
    }

    bool ok = echoes.size() == 2 &&
        std::fabs(echoes[1].distanceM - wallM) < 0.05f;

    std::printf("Acoustic ranging: %s\n", ok ? "PASS" : "FAIL");

    return ok ? 0 : 1;
}