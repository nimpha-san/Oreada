#include "wokwi-api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef struct {
  uart_dev_t uart;
  uint32_t co2_ppm;
  uint8_t buffer[9];
  uint8_t idx;
  uint32_t tick;
} chip_state_t;

static void on_uart_rx_data(void *user_data, uint8_t byte) {
  chip_state_t *chip = (chip_state_t *)user_data;
  chip->buffer[chip->idx++] = byte;
  
  if (chip->idx == 9) {
    chip->idx = 0;
    if (chip->buffer[0] == 0xFF && chip->buffer[1] == 0x01 && chip->buffer[2] == 0x86) {
      chip->tick++;
      chip->co2_ppm = (uint32_t)(1200 + 800 * sin(chip->tick * 0.3));
      
      uint8_t response[9];
      response[0] = 0xFF;
      response[1] = 0x86;
      response[2] = (chip->co2_ppm >> 8) & 0xFF;
      response[3] = chip->co2_ppm & 0xFF;
      response[4] = 0x00;
      response[5] = 0x00;
      response[6] = 0x00;
      response[7] = 0x00;
      
      uint8_t crc = 0;
      for (int i = 1; i < 8; i++) crc += response[i];
      response[8] = 0xFF - crc + 1;
      
      uart_write(chip->uart, response, 9);
      printf("[MH-Z19B] Отправлено: %d ppm\n", chip->co2_ppm);
    }
  }
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->co2_ppm = 800;
  chip->idx = 0;
  chip->tick = 0;
  
  // ВОТ ЗДЕСЬ ПЕРЕИМЕНОВЫВАЕМ IN/OUT В TX/RX
  uart_config_t uart_config = {
    .tx = pin_init("IN", INPUT_PULLUP),
    .rx = pin_init("OUT", INPUT),
    .baud_rate = 9600,
    .rx_data = on_uart_rx_data,
    .user_data = chip,
  };
  chip->uart = uart_init(&uart_config);
  
  printf("[MH-Z19B] Чип инициализирован\n");
}