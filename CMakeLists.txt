cmake_minimum_required(VERSION 3.16)
project(echolocate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- DSP core: platform-independent, no audio-device code, no dependencies ----
add_library(echo_core
    src/core/fft.cpp
    src/core/chirp.cpp
    src/core/ranging.cpp)
target_include_directories(echo_core PUBLIC src/core)
if(NOT MSVC)
    target_compile_options(echo_core PRIVATE -Wall -Wextra -O2)
endif()

# ---- Tests: synthetic echoes, no mic or speaker needed ----
enable_testing()
add_executable(core_test tests/core_test.cpp)
target_link_libraries(core_test PRIVATE echo_core)
add_test(NAME core_test COMMAND core_test)

# ---- Desktop app: full-duplex ping via miniaudio (fetched at configure time) ----
option(ECHO_BUILD_APP "Build the desktop mic+speaker app" ON)
if(ECHO_BUILD_APP)
    include(FetchContent)
    FetchContent_Declare(miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio.git
        GIT_TAG 0.11.21)
    FetchContent_MakeAvailable(miniaudio)

    add_executable(echo_app src/app/main.cpp)
    target_link_libraries(echo_app PRIVATE echo_core miniaudio)
endif()
