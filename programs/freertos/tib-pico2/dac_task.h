//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef DAC_TASK_H
#define DAC_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void dac_task(void *param);

#ifdef __cplusplus
}
#endif


#endif //DAC_TASK_H
