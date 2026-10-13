# echolocate 🦇

Active sonar on a speaker + mic: play a chirp, record, matched-filter, read echoes as distance.

## Layout
- `src/core/`  platform-independent DSP (chirp, FFT, matched filter, peak/range). No audio-device code.
- `src/app/`   desktop harness using miniaudio (full-duplex).
- `tests/`     synthetic-echo test, runs with no hardware.

## Build (CLion or CLI)
    cmake -B build && cmake --build build
    ./build/core_test     # should print PASS
    ./build/echo_app      # point the speaker at a wall, compare against a tape measure

## Android plan
Keep `src/core` untouched; swap `src/app` for an Oboe full-duplex stream behind JNI with a Kotlin UI.
Use the UNPROCESSED input source so echo cancellation doesn't delete the signal.
