//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef PHOTODIODE_TASK_H
#define PHOTODIODE_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

    void photodiode_task(void *param);

#ifdef __cplusplus
}
#endif


#endif //PHOTODIODE_TASK_H

