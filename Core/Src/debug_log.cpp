#include "tensorflow/lite/micro/debug_log.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Use 'extern' to point to the variables in main.c
extern "C" {
    extern uint8_t ring_buffer[];
    extern uint32_t head;
    extern uint32_t tail;
    extern bool dma_busy;
    extern UART_HandleTypeDef huart1;
}

#define TX_BUF_SIZE 2048

extern "C" void DebugLog(const char* format, va_list args) {
  char temp_buf[256];
  int len = vsnprintf(temp_buf, sizeof(temp_buf), format, args);

  if (len <= 0) return;

  // Copy into the shared ring buffer
  for (int i = 0; i < len; i++) {
    uint32_t next_head = (head + 1) % TX_BUF_SIZE;
    if (next_head != tail) {
      ring_buffer[head] = (uint8_t)temp_buf[i];
      head = next_head;
    }
  }

  // Handle the newline (\r\n) by adding it to the ring buffer too!
  // This prevents the CPU from blocking.
  const char* newline = "\r\n";
  for (int i = 0; i < 2; i++) {
    uint32_t next_head = (head + 1) % TX_BUF_SIZE;
    if (next_head != tail) {
      ring_buffer[head] = (uint8_t)newline[i];
      head = next_head;
    }
  }

  // Trigger DMA if idle
  if (!dma_busy && head != tail) {
    dma_busy = true;
    uint32_t send_size = (head > tail) ? (head - tail) : (TX_BUF_SIZE - tail);
    HAL_UART_Transmit_DMA(&huart1, &ring_buffer[tail], send_size);
  }
}