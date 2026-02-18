cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11) # TFLM requires at least C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)



# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME B-U585I-IOT02A_tinyML_IMU)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)


# Compiler options
set(STM32_MCU_FLAGS  "-mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mcmse " )

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    TF_LITE_STATIC_MEMORY  # Essential for Micro
    CMSIS_NN
)

file(GLOB_RECURSE TFLM_SOURCES
      "Middlewares/LiteRT/signal/*.cc" "Middlewares/LiteRT/signal/*.c"
      "Middlewares/LiteRT/tensorflow/*.cc"
      "Middlewares/LiteRT/third_party/*.c" "Middlewares/LiteRT/third_party/*.cc"
      "Drivers/BSP/Components/ism330dhcx/*.c")

list(REMOVE_ITEM TFLM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/LiteRT/tensorflow/lite/micro/debug_log.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/LiteRT/tensorflow/lite/micro/micro_time.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/LiteRT/tensorflow/lite/micro/system_setup.cc"
    
)


# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/app_main.cpp
    Core/Src/debug_log.cpp
    Core/Src/micro_time.cpp
    Core/Src/system_setup.cpp
    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.c
    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.c
    Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.c
    ${TFLM_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Inc
    # Go UP one level from 'Secure' to find 'Middleware'
    Middlewares/LiteRT
    Middlewares/LiteRT/third_party/flatbuffers/include
    Middlewares/LiteRT/third_party/gemmlowp
    Middlewares/LiteRT/third_party/kissfft
    Middlewares/LiteRT/third_party/ruy
    Middlewares/LiteRT/signal/src
    Middlewares/LiteRT/signal/src/kiss_fft_wrappers
    Middlewares/LiteRT/signal/micro
    Drivers/BSP/Components/ism330dhcx
    Drivers/BSP/B-U585I-IOT02A
    Drivers/BSP/Components/Common
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
