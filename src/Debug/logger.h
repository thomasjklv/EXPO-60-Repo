#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

void logger_init(void);
void logger_write(double time,
                  float yaw,
                  float pitch,
                  float gx,
                  float gy,
                  float gz);

void logger_close(void);

#endif