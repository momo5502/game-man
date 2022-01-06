cmake_minimum_required(VERSION 3.4.1)  # for example

add_library(gameboy_core  SHARED
    ../ios/Classes/egl_thread_surface.cpp
    ../ios/Classes/flutter_window.cpp
    ../ios/Classes/texture_2d.cpp
    ../ios/Classes/gameboy_core.cpp
    ../ios/Classes/gb/cpu.cpp
    ../ios/Classes/gb/game_boy.cpp
    ../ios/Classes/gb/gpu.cpp
    ../ios/Classes/gb/input.cpp
    ../ios/Classes/gb/mmu.cpp
    ../ios/Classes/gb/timer.cpp
    ../ios/Classes/gb/utils/utils.cpp
)

target_link_libraries(gameboy_core PUBLIC log android GLESv2 EGL)