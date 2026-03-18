#include "logger.h"
#include "config.h"

#include <stdio.h>
#include <stdbool.h>    
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct
{
    char name[LOGGER_MAX_NAME_LEN];
    char value[LOGGER_MAX_VALUE_LEN];
} logger_column_t;

static FILE *logfile = NULL;
static int header_written = 0;
static int current_column_count = 0;
static logger_column_t current_row[LOGGER_MAX_COLUMNS];
static uint64_t last_log_us = 0;

static uint64_t logger_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL +
           (uint64_t)(ts.tv_nsec / 1000ULL);
}

static void logger_make_filename(char *buffer, size_t size)
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

static void logger_write_header(void)
{
    if (!logfile || header_written || current_column_count <= 0) {
        return;
    }

    for (int i = 0; i < current_column_count; i++) {
        fprintf(logfile, "%s", current_row[i].name);

        if (i < current_column_count - 1) {
            fputc(',', logfile);
        }
    }

    fputc('\n', logfile);
    header_written = 1;

#if LOG_AUTO_FLUSH
    fflush(logfile);
#endif
}

static void logger_add_value(const char *name, const char *value)
{
    if (current_column_count >= LOGGER_MAX_COLUMNS) {
        return;
    }

    if (name == NULL || value == NULL) {
        return;
    }

    snprintf(current_row[current_column_count].name,
             LOGGER_MAX_NAME_LEN,
             "%s",
             name);

    snprintf(current_row[current_column_count].value,
             LOGGER_MAX_VALUE_LEN,
             "%s",
             value);

    current_column_count++;
}

bool logger_is_ready(void)
{
    return (logfile != NULL);
}

void logger_init(void)
{
    mkdir(LOG_FOLDER, 0777);

    char filename[256];
    logger_make_filename(filename, sizeof(filename));

    logfile = fopen(filename, "w");

    if (!logfile) {
        printf("LOGGER ERROR: could not open file: %s\n", filename);
        return;
    }

    printf("Logging to: %s\n", filename);

    header_written = 0;
    current_column_count = 0;
    last_log_us = 0;
}

void logger_close(void)
{
    if (logfile) {
        fclose(logfile);
        logfile = NULL;
    }

    header_written = 0;
    current_column_count = 0;
    last_log_us = 0;
}

void logger_begin_row(void)
{
    current_column_count = 0;
}

void logger_add_float(const char *name, float value)
{
    char buffer[LOGGER_MAX_VALUE_LEN];
    snprintf(buffer, sizeof(buffer), "%.6f", value);
    logger_add_value(name, buffer);
}

void logger_add_double(const char *name, double value)
{
    char buffer[LOGGER_MAX_VALUE_LEN];
    snprintf(buffer, sizeof(buffer), "%.6f", value);
    logger_add_value(name, buffer);
}

void logger_add_int(const char *name, int value)
{
    char buffer[LOGGER_MAX_VALUE_LEN];
    snprintf(buffer, sizeof(buffer), "%d", value);
    logger_add_value(name, buffer);
}

void logger_add_string(const char *name, const char *value)
{
    logger_add_value(name, value);
}

void logger_end_row(void)
{
    if (!logfile || current_column_count <= 0) {
        return;
    }

    const uint64_t now_us = logger_time_us();
    const uint64_t interval_us = 1000000ULL / LOG_RATE_HZ;

    if ((now_us - last_log_us) < interval_us) {
        return;
    }

    last_log_us = now_us;

    if (!header_written) {
        logger_write_header();
    }

    for (int i = 0; i < current_column_count; i++) {
        fprintf(logfile, "%s", current_row[i].value);

        if (i < current_column_count - 1) {
            fputc(',', logfile);
        }
    }

    fputc('\n', logfile);

#if LOG_AUTO_FLUSH
    fflush(logfile);
#endif
}