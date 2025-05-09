//
// Created by Jeb Bailey on 5/9/25.
//

#ifndef LOG_UTIL_H
#define LOG_UTIL_H

// #include <stdio.h>
// #include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t global_log_mutex;

// Get just the filename from __FILE__
static inline const char* __short_file__(const char* path) {
    const char* slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

#define SAFE_PRINTF(fmt, ...)                                     \
do {                                                                     \
    xSemaphoreTake(global_log_mutex, portMAX_DELAY);                                \
    printf("[%s:%d] " fmt, __short_file__(__FILE__), __LINE__, ##__VA_ARGS__);           \
    xSemaphoreGive(global_log_mutex);                                               \
} while (0)

#endif //LOG_UTIL_H
