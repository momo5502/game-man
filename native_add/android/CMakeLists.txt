cmake_minimum_required(VERSION 3.4.1)  # for example

add_library(native_add  SHARED
    ../ios/Classes/egl_thread_surface.cpp
    ../ios/Classes/native_add.cpp
    ../ios/Classes/gb/cpu.cpp
    ../ios/Classes/gb/game_boy.cpp
    ../ios/Classes/gb/gpu.cpp
    ../ios/Classes/gb/input.cpp
    ../ios/Classes/gb/mmu.cpp
    ../ios/Classes/gb/timer.cpp
    ../ios/Classes/gb/utils/utils.cpp
)

target_link_libraries(native_add PUBLIC log android GLESv2 EGL)