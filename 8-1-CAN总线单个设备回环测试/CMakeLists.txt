cmake_minimum_required(VERSION 3.20)

# CLion editing/indexing only. Build, flash and debug with Project.uvprojx.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(STM32F103_CAN_Loopback LANGUAGES C)

# Include new lab source files automatically after CMake reload.
file(GLOB_RECURSE index_sources CONFIGURE_DEPENDS
    "User/*.c"
    "Hardware/*.c"
    "System/*.c"
    "Library/*.c"
)
list(APPEND index_sources Start/system_stm32f10x.c)
file(GLOB_RECURSE index_headers CONFIGURE_DEPENDS
    "User/*.h"
    "Hardware/*.h"
    "System/*.h"
    "Library/*.h"
    "Start/*.h"
)

# ARM/Keil assembly stays available for reading, outside host compilation.
set(browse_only_sources
    Start/core_cm3.c
    Start/startup_stm32f10x_md.s
)
set_source_files_properties(${browse_only_sources} PROPERTIES HEADER_FILE_ONLY TRUE)

# OBJECT supplies real per-file settings to CLion, without a firmware link.
# Excluded from ALL so the default build does not compile embedded sources.
add_library(CLionIndex OBJECT EXCLUDE_FROM_ALL
    ${index_sources}
    ${index_headers}
    ${browse_only_sources}
)
set_target_properties(CLionIndex PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
)
target_compile_definitions(CLionIndex PRIVATE
    STM32F10X_MD
    USE_STDPERIPH_DRIVER
    CLION_CODE_INDEX=1
)
target_include_directories(CLionIndex PRIVATE
    Start
    Library
    User
    System
    Hardware
)

# Match STM32's 32-bit pointer model when indexing with bundled MinGW.
if(MINGW AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_options(CLionIndex PRIVATE -m32)
endif()

message(STATUS "CLion indexing only: build/flash/debug with Project.uvprojx in Keil")
