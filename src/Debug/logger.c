#include "logger.h"
#include "config.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

static FILE *logfile = NULL;
static uint64_t last_log_us = 0;

static uint64_t time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL +
           (uint64_t)(ts.tv_nsec / 1000ULL);
}

static void make_filename(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    snprintf(buffer, size,
             "%slog_%04d-%02d-%02d_%02d-%02d-%02d.csv",
             LOG_FOLDER,
             t->tm_year + 1900,
             t->tm_mon + 1,
             t->tm_mday,
             t->tm_hour,
             t->tm_min,
             t->tm_sec);
}

void logger_init(void)
{
    mkdir(LOG_FOLDER, 0777);

    char filename[256];
    make_filename(filename, sizeof(filename));

    logfile = fopen(filename, "w");

    if (!logfile)
    {
        printf("LOGGER ERROR: could not open file: %s\n", filename);
        return;
    }

    printf("Logging to: %s\n", filename);

    fprintf(logfile, "time,yaw,pitch,gyro_x,gyro_y,gyro_z\n");
    fflush(logfile);

    last_log_us = 0;
}

void logger_write(double time,
                  float yaw,
                  float pitch,
                  float gx,
                  float gy,
                  float gz)
{
    if (!logfile)
        return;

    const uint64_t now = time_us();
    const uint64_t interval_us = 1000000ULL / LOG_RATE_HZ;

    if ((now - last_log_us) < interval_us)
        return;

    last_log_us = now;

    fprintf(logfile,
            "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
            time, yaw, pitch, gx, gy, gz);

#if LOG_FLUSH_INTERVAL
    fflush(logfile);
#endif
}

void logger_close(void)
{
    if (logfile)
    {
        fclose(logfile);
        logfile = NULL;
    }
}