#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>
#include "vector3.h"

typedef struct 
{
    bool ARMED;
    vector3 gyro_RAD;
    vector3 comps_RAD;
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

/* ================= CONVERSIONS ================= */
float rad_to_deg(float rad);    

/* ================= GPS ================= */
gpsSTR get_GPS(void);


#endif