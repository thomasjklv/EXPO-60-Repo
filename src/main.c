/*
===============================================================================
  ████████╗ ██████╗ ██████╗ ██████╗ ███████╗██████╗  ██████╗
  ╚══██╔══╝██╔═══██╗██╔══██╗██╔══██╗██╔════╝██╔══██╗██╔═══██╗
     ██║   ██║   ██║██████╔╝██████╔╝█████╗  ██║  ██║██║   ██║
     ██║   ██║   ██║██╔══██╗██╔═══╝ ██╔══╝  ██║  ██║██║   ██║
     ██║   ╚██████╔╝██║  ██║██║     ███████╗██████╔╝╚██████╔╝
     ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚══════╝╚═════╝  ╚═════╝

===============================================================================
  Project 60 – Underwater Drone Defence
  Expo Engineering Project

  Platform:
    • Raspberry Pi 3 Model B
    • Pixhawk 4 (MAVLink controlled)

  Description:
    Autonomous / semi-autonomous underwater defence drone control software.
    Handles sensor acquisition, MAVLink communication, actuator control,
    and real-time system feedback.

  Configuration:
    Adjust system parameters and hardware settings in:
      config.h

  Authors:
    Thomas van Bakel  – thomas.vanbakel@student.fontys.nl
    Lucas de Leur     – l.deleur@student.fontys.nl

  Institution:
    Fontys University of Applied Sciences

===============================================================================
*/



#pragma region Includes
// Include all Lib includes here Ex:<stdio.h>
/*============ Pi-Lib Files ============*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include "common/mavlink.h"

// Include the config file here
/*======== Config settings File ========*/
#include "config.h"

// Include Custom modules here Ex:"Your Module.h"
/*=========== Custom Modules ===========*/
#include "common_Control/actuators.h"
#include "common_Control/control.h"
#include "common_Control/vector3.h"
#include "Debug/logger.h"
// Define this script only Defines Ex: #define value true
/*=========== Script Defines ===========*/
#define LISTEN_PORT 14670

#pragma endregion

/*=========== DRONE CONTROL ===========*/
volatile drone_MAIN TOP_DRONE = {.ARMED = 0};

/*=============== SERVOS ==============*/
srvSTR SERVO_TEST = {    
    .MAX_ANGLE = 90,
    .MIN_ANGLE = 0,
    .DFLT_ANGLE = 45,
    .ANGLE = 90,
    .CHANNEL = 8
};

void EXIT_TASK(int sig)
{
    printf("\nEXIT\n");
    disarmDrone();
    logger_close();
    exit(1);
}


double get_time_s()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// MAIN TELEMETRY THREAD: This Thread is made for reading and setting Telemetry Values
void* thread_1_Telemetry(void* arg)
{
    while (1)
    {

        TOP_DRONE.gyro_RAD = get_GYRO_V3();
        TOP_DRONE.comps_RAD = get_COMPS_V3();
        TOP_DRONE.gps = get_GPS();

        double t = get_time_s();

        logger_write(t, get_YAW_HEADING(),get_PITCH_HEADING(),
        TOP_DRONE.gyro_RAD.x, TOP_DRONE.gyro_RAD.y, TOP_DRONE.gyro_RAD.z);


        if (sPrintTelemetry){
        printf("\rYaw:%6.2f deg  Pitch:%6.2f deg  |  Gyro X:%6.3f Y:%6.3f Z:%6.3f  |  GPS: %.6f %.6f Alt: %.2f m   ",
               get_YAW_HEADING(),
               get_PITCH_HEADING(),
               TOP_DRONE.gyro_RAD.x, TOP_DRONE.gyro_RAD.y, TOP_DRONE.gyro_RAD.z,
               TOP_DRONE.gps.lat_deg, TOP_DRONE.gps.lon_deg, TOP_DRONE.gps.alt_m);
        fflush(stdout);
        }
    }
    return NULL;
}

// MAIN CONTROL THREAD: This Thread is made for controlling the drones actuators
void* thread_2_Control(void* arg)
{
    while (1)
    { 
        set_ServoAngle(&SERVO_TEST, SERVO_TEST.MIN_ANGLE);
        sleep(1);
        set_ServoAngle(&SERVO_TEST, SERVO_TEST.MAX_ANGLE); 
        sleep(1);
    }
    return NULL;
}



int main(void)
{
    logger_init();

    signal(SIGINT, EXIT_TASK);
    disarmDrone();
    if (AUTOARM) {
        printf("WARNING: DRONE WILL AUTO ARM IN 5s...\n");
        sleep(5);
        armDrone(); TOP_DRONE.ARMED = true; printf("DRONE ARMED\n");
    }
    
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread_1_Telemetry, NULL);
    pthread_create(&t2, NULL, thread_2_Control, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}