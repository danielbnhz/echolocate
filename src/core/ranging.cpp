
#include "ranging.h"
#include "fft.h"
#include "bitwise/bitops.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace echo {

std::vector<float> matchedFilter(const std::vector<float>& rec,
                                 const std::vector<float>& chirp) {
    if (rec.empty() || chirp.empty()) return {};

    const size_t n = nextPow2(rec.size() + chirp.size() - 1);

    std::vector<cpx> R(n), C(n);

    for (size_t i = 0; i < rec.size(); ++i)
        R[i] = rec[i];

    for (size_t i = 0; i < chirp.size(); ++i)
        C[i] = chirp[i];

    fft(R);
    fft(C);

    for (size_t i = 0; i < n; ++i)
        R[i] *= std::conj(C[i]);

    fft(R, true);

    std::vector<float> out(rec.size());

    for (size_t i = 0; i < rec.size(); ++i)
        out[i] = std::abs(R[i]);

    return out;
}

namespace {

// Original scalar candidate detection.
std::vector<size_t> collectPeaksScalar(
    const std::vector<float>& mf,
    float thresh,
    size_t minSep)
{
    std::vector<size_t> peaks;

    for (size_t i = 1; i + 1 < mf.size(); ++i) {
        if (mf[i] >= thresh &&
            mf[i] > mf[i - 1] &&
            mf[i] >= mf[i + 1]) {

            if (!peaks.empty() && i - peaks.back() < minSep) {
                if (mf[i] > mf[peaks.back()])
                    peaks.back() = i;
            } else {
                peaks.push_back(i);
            }
        }
    }

    return peaks;
}

// Bitwise candidate detection.
std::vector<size_t> collectPeaksBitwise(
    const std::vector<float>& mf,
    float thresh,
    size_t minSep)
{
    std::vector<size_t> peaks;

    constexpr size_t BITS_PER_MASK = 64;

    // Each block represents up to 64 consecutive samples.
    for (size_t blockStart = 1;
         blockStart < mf.size() - 1;
         blockStart += BITS_PER_MASK)
    {
        uint64_t mask = 0;

        const size_t remaining = (mf.size() - 1) - blockStart;
        const size_t blockLength =
            std::min(BITS_PER_MASK, remaining);

        // First pass: identify candidates and set their bits.
        for (size_t offset = 0; offset < blockLength; ++offset) {
            const size_t i = blockStart + offset;

            if (mf[i] >= thresh &&
                mf[i] > mf[i - 1] &&
                mf[i] >= mf[i + 1])
            {
                mask = bitwise::setBit(
                    mask,
                    static_cast<unsigned>(offset)
                );
            }
        }

        // Second pass: enumerate only the set bits.
        while (mask != 0) {
            const unsigned bit =
                bitwise::countTrailingZeros(mask);

            const size_t i = blockStart + bit;

            if (!peaks.empty() && i - peaks.back() < minSep) {
                if (mf[i] > mf[peaks.back()])
                    peaks.back() = i;
            } else {
                peaks.push_back(i);
            }

            mask = bitwise::clearLowestSetBit(mask);
        }
    }

    return peaks;
}

// Shared conversion from candidate positions to Echo objects.
std::vector<Echo> buildEchoes(
    const std::vector<float>& mf,
    const std::vector<size_t>& peaks,
    float sampleRate)
{
    std::vector<Echo> result;

    if (peaks.empty())
        return result;

    const float maxv =
        *std::max_element(mf.begin(), mf.end());

    // Preserve the original strongest-peak reference.
    size_t t0 = peaks.front();

    for (size_t p : peaks) {
        if (mf[p] > mf[t0])
            t0 = p;
    }

    for (size_t p : peaks) {
        Echo e;

        e.lagSamples = p;
        e.strength = mf[p] / maxv;

        // Use signed arithmetic to prevent unsigned underflow.
        const double sampleDelta =
            static_cast<double>(p) - static_cast<double>(t0);

        e.distanceM = static_cast<float>(
            sampleDelta / sampleRate *
            kSpeedOfSound * 0.5
        );

        result.push_back(e);
    }

    return result;
}

} // anonymous namespace

// Baseline implementation.
std::vector<Echo> findEchoes(
    const std::vector<float>& mf,
    float sampleRate,
    float relThreshold,
    size_t minSep)
{
    if (mf.size() < 3) return {};

    const float maxv =
        *std::max_element(mf.begin(), mf.end());

    if (maxv <= 0.0f) return {};

    const float thresh = maxv * relThreshold;

    const auto peaks =
        collectPeaksScalar(mf, thresh, minSep);

    return buildEchoes(mf, peaks, sampleRate);
}

// Experimental bitwise implementation.
std::vector<Echo> findEchoesBitwise(
    const std::vector<float>& mf,
    float sampleRate,
    float relThreshold,
    size_t minSep)
{
    if (mf.size() < 3) return {};

    const float maxv =
        *std::max_element(mf.begin(), mf.end());

    if (maxv <= 0.0f) return {};

    const float thresh = maxv * relThreshold;

    const auto peaks =
        collectPeaksBitwise(mf, thresh, minSep);

    return buildEchoes(mf, peaks, sampleRate);
}

} // namespace echo
