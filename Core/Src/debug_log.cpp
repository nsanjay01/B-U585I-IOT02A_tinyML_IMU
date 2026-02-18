#include "tensorflow/lite/micro/debug_log.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart1;

extern "C" void DebugLog(const char* format, va_list args) {
  char buffer[256];
  
  // 1. Format the string into our buffer
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  
  // 2. Transmit the formatted string via UART
  if (len > 0) {
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, len, HAL_MAX_DELAY);
  }
  
  // 3. Add a newline for readability
  HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
}