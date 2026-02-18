#include "tensorflow/lite/micro/micro_time.h"
#include "main.h" // Gives access to htim2

namespace tflite {

// The profiler calls this to measure performance
uint32_t GetCurrentTimeTicks() {
    return HAL_GetTick(); 
}

// Some versions also require this
uint32_t ticks_per_second() {
    return 1000; 
}

}