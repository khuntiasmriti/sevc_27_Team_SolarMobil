#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "main.h"

#define WHEEL_DIAMETER_METERS   0.50f
#define GEAR_REDUCTION_RATIO    1.00f

typedef struct
{
  uint32_t id;
  uint8_t extended;
  uint16_t rpm;
  float speed_kmh;
  float controller_temp_c;
  float motor_temp_c;
  uint8_t dlc;
  uint8_t data[8];
  uint32_t rx_count;
  uint8_t valid;
} MotorCanData_t;

void Motor_Init(void);
void Motor_PollReception(void);
const MotorCanData_t* Motor_GetData(void);

#endif /* MOTOR_CONTROLLER_H */
