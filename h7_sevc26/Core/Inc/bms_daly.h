#ifndef BMS_DALY_H
#define BMS_DALY_H

#include "main.h"

typedef struct
{
  float pack_voltage_v;    /* HV Actual Voltage */
  float pack_current_a;    /* HV Charge/Discharge Current */
  float soc_percent;       /* HV State of Charge (%) */
  float soh_percent;       /* HV State of Health (%) */
  float capacity_ah;       /* HV Battery Capacity (Ah) */
  float max_temp_c;        /* HV Battery Temperature (°C) */
  uint8_t valid;
  uint32_t good_frames;
  uint32_t bad_frames;
} DalyData_t;

void BMS_Init(void);
void BMS_Poll(void);
const DalyData_t* BMS_GetData(void);

#endif /* BMS_DALY_H */
