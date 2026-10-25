#include "bms_daly.h"

#define DALY_QUERY_PERIOD_MS       500U
#define DALY_RESPONSE_TIMEOUT_MS   100U

extern UART_HandleTypeDef huart4;

static volatile DalyData_t daly = {0};
static uint8_t daly_rx_frame[13];
static uint32_t last_daly_query_ms = 0;

static uint8_t DALY_Checksum(const uint8_t *buf, uint16_t len)
{
  uint8_t sum = 0U;
  for (uint16_t i = 0; i < len; i++)
  {
    sum = (uint8_t)(sum + buf[i]);
  }
  return sum;
}

static uint8_t DALY_ReadPackMeasurements(void)
{
  /* 0x90 Command: Pack Voltage, Current, SOC */
  uint8_t tx[13] = {0xA5, 0x40, 0x90, 0x08, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  tx[12] = DALY_Checksum(tx, 12U);

  __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_OREF | UART_CLEAR_FEF | UART_CLEAR_NEF);
  __HAL_UART_SEND_REQ(&huart4, UART_RXDATA_FLUSH_REQUEST);

  if (HAL_UART_Transmit(&huart4, tx, sizeof(tx), 50U) != HAL_OK)
  {
    return 0U;
  }

  if (HAL_UART_Receive(&huart4, daly_rx_frame, sizeof(daly_rx_frame), DALY_RESPONSE_TIMEOUT_MS) != HAL_OK)
  {
    daly.bad_frames++;
    return 0U;
  }

  if ((daly_rx_frame[0] != 0xA5U) || (daly_rx_frame[1] != 0x01U) ||
      (daly_rx_frame[2] != 0x90U) || (daly_rx_frame[3] != 0x08U) ||
      (DALY_Checksum(daly_rx_frame, 12U) != daly_rx_frame[12]))
  {
    daly.bad_frames++;
    return 0U;
  }

  uint16_t voltage_raw = ((uint16_t)daly_rx_frame[4] << 8) | daly_rx_frame[5];
  uint16_t current_raw = ((uint16_t)daly_rx_frame[8] << 8) | daly_rx_frame[9];
  uint16_t soc_raw     = ((uint16_t)daly_rx_frame[10] << 8) | daly_rx_frame[11];

  daly.pack_voltage_v = (float)voltage_raw / 10.0f;
  daly.pack_current_a = ((float)current_raw - 30000.0f) / 10.0f;
  daly.soc_percent    = (float)soc_raw / 10.0f;

  /* Standard Daly default values for SOH and Capacity if 0x92 query is omitted */
  daly.soh_percent    = 100.0f;
  daly.capacity_ah    = 100.0f;

  daly.valid = 1U;
  daly.good_frames++;
  return 1U;
}

void BMS_Init(void)
{
  last_daly_query_ms = HAL_GetTick();
}

void BMS_Poll(void)
{
  uint32_t now = HAL_GetTick();
  if ((uint32_t)(now - last_daly_query_ms) >= DALY_QUERY_PERIOD_MS)
  {
    last_daly_query_ms = now;
    (void)DALY_ReadPackMeasurements();
  }
}

const DalyData_t* BMS_GetData(void)
{
  return (const DalyData_t*)&daly;
}
