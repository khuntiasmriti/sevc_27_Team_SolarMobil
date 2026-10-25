/*
 * esp32_telemetry.c
 *
 *  Created on: 23-Oct-2026
 *      Author: khunt
 */
#include "esp32_telemetry.h"
#include "motor_controller.h"
#include "bms_daly.h"

#define ESP32_TX_PERIOD_MS   100U
#define TELEMETRY_PACKET_MAX 32U

extern UART_HandleTypeDef huart7;

static uint8_t telemetry_packet[TELEMETRY_PACKET_MAX];
static uint16_t telemetry_packet_len = 0;
static uint32_t last_esp32_tx_ms = 0;

static void Telemetry_BuildPacket(void)
{
  const MotorCanData_t *motor = Motor_GetData();
  const DalyData_t *bms = BMS_GetData();

  int32_t current_tenths = 0;
  uint16_t voltage_tenths = 0U, soc_tenths = 0U;

  telemetry_packet[0] = 0x54U; /* Header 'T' */
  telemetry_packet[1] = 1U;    /* Protocol Version */
  telemetry_packet[2] = motor->valid;
  telemetry_packet[3] = (uint8_t)(motor->id & 0xFFU);
  telemetry_packet[4] = (uint8_t)((motor->id >> 8) & 0xFFU);
  telemetry_packet[5] = (uint8_t)((motor->id >> 16) & 0xFFU);
  telemetry_packet[6] = (uint8_t)((motor->id >> 24) & 0xFFU);
  telemetry_packet[7] = motor->extended;
  telemetry_packet[8] = motor->dlc;

  for (uint8_t i = 0; i < 8U; i++)
  {
    telemetry_packet[9U + i] = motor->data[i];
  }

  telemetry_packet[17] = bms->valid;
  if (bms->valid)
  {
    voltage_tenths = (uint16_t)(bms->pack_voltage_v * 10.0f);
    current_tenths = (int32_t)(bms->pack_current_a * 10.0f);
    soc_tenths     = (uint16_t)(bms->soc_percent * 10.0f);
  }

  telemetry_packet[18] = (uint8_t)(voltage_tenths & 0xFFU);
  telemetry_packet[19] = (uint8_t)(voltage_tenths >> 8);
  telemetry_packet[20] = (uint8_t)(current_tenths & 0xFF);
  telemetry_packet[21] = (uint8_t)((current_tenths >> 8) & 0xFF);
  telemetry_packet[22] = (uint8_t)(soc_tenths & 0xFFU);
  telemetry_packet[23] = (uint8_t)(soc_tenths >> 8);
  telemetry_packet_len = 24U;
}

void Telemetry_Init(void)
{
  last_esp32_tx_ms = HAL_GetTick();
}

void Telemetry_PollTransmit(void)
{
  uint32_t now = HAL_GetTick();
  if ((uint32_t)(now - last_esp32_tx_ms) >= ESP32_TX_PERIOD_MS)
  {
    last_esp32_tx_ms = now;
    Telemetry_BuildPacket();
    (void)HAL_UART_Transmit(&huart7, telemetry_packet, telemetry_packet_len, 20U);
  }
}
