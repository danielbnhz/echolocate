cmake_minimum_required(VERSION 3.16)
project(echolocate LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library(echo_core
    src/core/fft.cpp
    src/core/chirp.cpp
    src/core/ranging.cpp
        src/core/bitwise/bitops.cpp
        src/core/bitwise/bitops.h)
target_include_directories(echo_core PUBLIC src/core)
if(NOT MSVC)
    target_compile_options(echo_core PRIVATE -Wall -Wextra -O2)
endif()


enable_testing()
add_executable(core_test tests/core_test.cpp
        src/core/bitwise/bitops.cpp
        src/core/bitwise/bitops.h)
target_link_libraries(core_test PRIVATE echo_core)
add_test(NAME core_test COMMAND core_test)

 option(ECHO_BUILD_APP "Build the desktop mic+speaker app" ON)
if(ECHO_BUILD_APP)
    include(FetchContent)
    FetchContent_Declare(miniaudio
            GIT_REPOSITORY https://github.com/mackron/miniaudio.git
            GIT_TAG 0.11.21
            SOURCE_SUBDIR do_not_build)
    FetchContent_MakeAvailable(miniaudio)

    add_executable(echo_app src/app/main.cpp
            src/core/bitwise/bitops.cpp
            src/core/bitwise/bitops.h)
    target_link_libraries(echo_app PRIVATE echo_core)
    target_include_directories(echo_app PRIVATE ${miniaudio_SOURCE_DIR})

    if(UNIX AND NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(echo_app PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
    endif()
endif()