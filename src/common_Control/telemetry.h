#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdbool.h>
#include "vector3.h"

typedef struct 
{
    bool ARMED;
    vector3 gyro_RAD;
    vector3 comps_RAD;
    vector3 accel_V3;
    float compsYAW;
    float compsPITCH;
    gpsSTR gps;
} drone_MAIN;

/* ================= DRONE ================= */
void armDrone(void);
void disarmDrone(void);

/* ================= GYRO ================= */
vector3 get_GYRO_V3(void);

/* ================= COMPASS ================= */
vector3 get_COMPS_V3(void);
float get_YAW_HEADING(void);
float get_PITCH_HEADING(void);

/* ================= Accel ================= */
vector3 get_ACCEL_V3(void);

/* ================= CONVERSIONS ================= */
float rad_to_deg(float rad);    

/* ================= GPS ================= */
gpsSTR get_GPS(void);


#endif