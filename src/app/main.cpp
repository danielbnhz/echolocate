// Desktop echolocation ping: plays a chirp while recording, then ranges echoes.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "chirp.h"
#include "ranging.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <thread>
#include <vector>

// Which capture channel is the mic? Zero-based: 0 = input 1, 1 = input 2.
constexpr ma_uint32 kMicChannel = 1;
// How many capture channels to open (2 for a two-input interface).
constexpr ma_uint32 kRequestedCaptureChannels = 2;
constexpr ma_uint32 kMaxChannels = 8;

struct PingState {
    std::vector<float> chirp;
    std::vector<float> recording;
    size_t playPos = 0;
    size_t recPos = 0;
    float chPeak[kMaxChannels] = {};
    std::atomic<bool> done{false};
};

static void dataCallback(ma_device* dev, void* out, const void* in, ma_uint32 frames) {
    auto* s = static_cast<PingState*>(dev->pUserData);
    auto* o = static_cast<float*>(out);
    auto* i = static_cast<const float*>(in);


    const ma_uint32 inCh  = dev->capture.channels;
    const ma_uint32 outCh = dev->playback.channels;
    const ma_uint32 mic   = (kMicChannel < inCh) ? kMicChannel : 0; // clamp, never overrun

    for (ma_uint32 f = 0; f < frames; ++f) {
        float sample = (s->playPos < s->chirp.size()) ? s->chirp[s->playPos++] : 0.0f;
        for (ma_uint32 c = 0; c < outCh; ++c) o[f * outCh + c] = sample;

        for (ma_uint32 c = 0; c < inCh && c < kMaxChannels; ++c) {
            float a = std::fabs(i[f * inCh + c]);
            if (a > s->chPeak[c]) s->chPeak[c] = a;
        }
        if (s->recPos < s->recording.size())
            s->recording[s->recPos++] = i[f * inCh + mic];
    }
    if (s->recPos >= s->recording.size()) s->done = true;
}

int main() {
    const ma_uint32 fs = 48000;
    PingState state;
    state.chirp = echo::makeChirp(2000.0f, 20000.0f, 0.004f, static_cast<float>(fs));
    state.recording.assign(fs * 3 / 10, 0.0f);

    ma_device_config cfg = ma_device_config_init(ma_device_type_duplex);
    cfg.sampleRate = fs;
    cfg.capture.format = ma_format_f32;  cfg.capture.channels = kRequestedCaptureChannels;
    cfg.playback.format = ma_format_f32; cfg.playback.channels = 1;
    cfg.dataCallback = dataCallback;
    cfg.pUserData = &state;

    ma_device device;
    if (ma_device_init(nullptr, &cfg, &device) != MA_SUCCESS) {
        std::fprintf(stderr, "failed to open audio device\n");
        return 1;
    }
    std::printf("opened: capture %u ch, playback %u ch, %u Hz\n",
                device.capture.channels, device.playback.channels, device.sampleRate);

    ma_device_start(&device);
    while (!state.done) std::this_thread::sleep_for(std::chrono::milliseconds(5));
    ma_device_uninit(&device);

    std::printf("input peaks: ch1=%.3f  ch2=%.3f\n", state.chPeak[0], state.chPeak[1]);

    auto mf = echo::matchedFilter(state.recording, state.chirp);
    auto echoes = echo::findEchoes(mf, static_cast<float>(fs));
    std::printf("found %zu peaks (first = direct path)\n", echoes.size());
    for (auto& e : echoes)
        std::printf("  lag=%zu  strength=%.2f  distance=%.2f m\n", e.lagSamples, e.strength, e.distanceM);
    return 0;
}