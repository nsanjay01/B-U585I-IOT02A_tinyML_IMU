#include "tensorflow/lite/micro/debug_log.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Use 'extern' to point to the variables in main.c
extern "C" {
    extern uint8_t ring_buffer[];
    volatile   extern uint32_t head;
    volatile  extern uint32_t tail;
    volatile   extern bool dma_busy;
    extern UART_HandleTypeDef huart1;
}

#define TX_BUF_SIZE 2048

extern "C" void DebugLog(const char* format, va_list args) {
  char temp_buf[256];
  // 1. Format the main message
  int len = vsnprintf(temp_buf, sizeof(temp_buf), format, args);
  if (len <= 0) return;

  // 2. Add the main message to the ring buffer
  for (int i = 0; i < len; i++) {
    uint32_t next_head = (head + 1) % TX_BUF_SIZE;
    if (next_head != tail) {
      ring_buffer[head] = (uint8_t)temp_buf[i];
      head = next_head;
    }
  }

  // 3. Kick off DMA only ONCE at the very end
  // Using a critical section (disabling interrupts) for a split second 
  // ensures head/tail don't move while we check dma_busy.
  __disable_irq(); 
  if (!dma_busy && head != tail) {
    dma_busy = true;
    uint32_t send_size = (head > tail) ? (head - tail) : (TX_BUF_SIZE - tail);
    if (HAL_UART_Transmit_DMA(&huart1, &ring_buffer[tail], send_size) != HAL_OK) {
        dma_busy = false; // Reset if DMA failed to start
    }
  }
  __enable_irq();
}