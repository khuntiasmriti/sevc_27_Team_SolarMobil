#include "motor_controller.h"
#include <string.h>
#include <math.h>

extern FDCAN_HandleTypeDef hfdcan1;
static volatile MotorCanData_t motor_can = {0};

void Motor_Init(void)
{
  FDCAN_FilterTypeDef sFilterConfig = {0};

  sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1 = 0x10F8108D;
  sFilterConfig.FilterID2 = 0x00000000;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,
                                   FDCAN_REJECT_REMOTE,
                                   FDCAN_REJECT_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
}

void Motor_PollReception(void)
{
  FDCAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0U)
  {
    memset(rx_data, 0, sizeof(rx_data));
    if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
    {
      break;
    }

    motor_can.id = rx_header.Identifier;
    motor_can.extended = (rx_header.IdType == FDCAN_EXTENDED_ID) ? 1U : 0U;
    for (uint8_t i = 0; i < 8U; i++)
    {
      motor_can.data[i] = rx_data[i];
    }

    /* FarDriver Frame Parsing for Extended CAN 0x10F8108D */
    /* RPM (Bytes 0-1 Big Endian) */
    motor_can.rpm = (uint16_t)((rx_data[0] << 8) | rx_data[1]);

    /* Calculate Speed (km/h) = (RPM * PI * Wheel_Diameter * 60) / (1000 * Gear_Ratio) */
    float speed_m_per_min = (float)motor_can.rpm * 3.14159f * WHEEL_DIAMETER_METERS / GEAR_REDUCTION_RATIO;
    motor_can.speed_kmh = (speed_m_per_min * 60.0f) / 1000.0f;

    /* Controller and Motor Temperatures (Offset -40°C) */
    motor_can.controller_temp_c = (float)rx_data[4] - 40.0f;
    motor_can.motor_temp_c      = (float)rx_data[5] - 40.0f;

    motor_can.rx_count++;
    motor_can.valid = 1U;
  }
}

const MotorCanData_t* Motor_GetData(void)
{
  return (const MotorCanData_t*)&motor_can;
}
