// Desktop echolocation ping: plays a chirp while recording, then ranges echoes.
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "chirp.h"
#include "ranging.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

struct PingState {
    std::vector<float> chirp;
    std::vector<float> recording;
    size_t playPos = 0;
    size_t recPos = 0;
    std::atomic<bool> done{false};
};

static void dataCallback(ma_device* dev, void* out, const void* in, ma_uint32 frames) {
    auto* s = static_cast<PingState*>(dev->pUserData);
    auto* o = static_cast<float*>(out);
    auto* i = static_cast<const float*>(in);

    for (ma_uint32 f = 0; f < frames; ++f) {
        o[f] = (s->playPos < s->chirp.size()) ? s->chirp[s->playPos++] : 0.0f;
        if (s->recPos < s->recording.size()) s->recording[s->recPos++] = i[f];
    }
    if (s->recPos >= s->recording.size()) s->done = true;
}

int main() {
    const ma_uint32 fs = 48000;
    PingState state;
    state.chirp = echo::makeChirp(2000.0f, 20000.0f, 0.004f, static_cast<float>(fs));
    state.recording.assign(fs / 10, 0.0f); // record 100 ms (~17 m of range)

    ma_device_config cfg = ma_device_config_init(ma_device_type_duplex);
    cfg.sampleRate = fs;
    cfg.capture.format = ma_format_f32;  cfg.capture.channels = 1;
    cfg.playback.format = ma_format_f32; cfg.playback.channels = 1;
    cfg.dataCallback = dataCallback;
    cfg.pUserData = &state;

    ma_device device;
    if (ma_device_init(nullptr, &cfg, &device) != MA_SUCCESS) {
        std::fprintf(stderr, "failed to open audio device\n");
        return 1;
    }
    ma_device_start(&device);
    while (!state.done) std::this_thread::sleep_for(std::chrono::milliseconds(5));
    ma_device_uninit(&device);

    auto mf = echo::matchedFilter(state.recording, state.chirp);
    auto echoes = echo::findEchoes(mf, static_cast<float>(fs));
    std::printf("found %zu peaks (first = direct path)\n", echoes.size());
    for (auto& e : echoes)
        std::printf("  lag=%zu  strength=%.2f  distance=%.2f m\n", e.lagSamples, e.strength, e.distanceM);
    return 0;
}
